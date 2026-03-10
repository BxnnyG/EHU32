# Known Issues and Solutions

> This document was compiled from community discussions in the GitHub Issues. See the original threads for full context.
> Original issues: [#1](https://github.com/PNKP237/EHU32/issues/1) · [#3](https://github.com/PNKP237/EHU32/issues/3) · [#6](https://github.com/PNKP237/EHU32/issues/6) · [#15](https://github.com/PNKP237/EHU32/issues/15) · [#16](https://github.com/PNKP237/EHU32/issues/16) · [#19](https://github.com/PNKP237/EHU32/issues/19)

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## Audio Issues

### Crackling, stuttering, or audio dropouts

**Affected devices:** iPhones and some Android phones (especially Huawei)

**Root cause:** Clone ESP32 modules — specifically boards using a **BoyaMicro** flash chip under the RF shield — have been confirmed to cause audio stuttering with iOS devices. This appears to be a hardware issue with the ESP32 chip or flash itself, not a firmware problem. ([Issue #6](https://github.com/PNKP237/EHU32/issues/6))

**Fixes:**
1. Replace the ESP32 board with one using a genuine **Espressif** module. Look for "Espressif" etched on the metal RF shield, not just printed text. See [community-knowledge.md](community-knowledge.md) for board identification.
2. If you have soldering skills: replace just the ESP-WROOM-32E module on the carrier board with a genuine one — this has been confirmed to fix the stuttering. ([Issue #6, maks013-dev](https://github.com/PNKP237/EHU32/issues/6#issuecomment-3726338115))
3. Boards with an external **IPX/U.FL antenna connector** generally fare better than boards with only a PCB antenna. **Avoid "ESP-32S" boards** — these have been found to be particularly unreliable.

**Recommended board:** WeAct Studio ESP32 D0WD v3 has been tested and confirmed working well with both iOS and Android. ([Issue #6, PNKP237](https://github.com/PNKP237/EHU32/issues/6#issuecomment-3190095186))

---

### Android "Keep volume consistent" causes stuttering or clipping

**Symptom:** Audio stutters or clips; Android phone's volume appears to adjust itself automatically during playback.

**Root cause:** EHU32 instructs the Bluetooth receiver to ignore volume control from the connected device. Android detects this and applies its own local volume adjustment before sending audio, reducing dynamic range and causing clipping at high volumes.

**Fix:** On Android, open **Settings → Bluetooth → tap on EHU32 → disable "Keep volume consistent"**. Set the phone volume to maximum for best audio quality. ([Issue #6, PNKP237](https://github.com/PNKP237/EHU32/issues/6#issuecomment-3086578823))

---

### iPhone volume mute doesn't stop audio

**Symptom:** Muting the iPhone still allows audio to play through EHU32.

**Explanation:** This is intentional behaviour. EHU32 disables remote volume control from the connected device, and iOS respects this by maintaining the audio stream regardless of the phone's mute state. ([Issue #6, PNKP237](https://github.com/PNKP237/EHU32/issues/6#issuecomment-3086578823))

---

### Ground hum or buzz (audio ground loop)

**Symptom:** A constant hum or buzz audible through the speakers when EHU32 is connected.

**Root cause:** The audio input ground (from the PCM5102A AUX cable) and the vehicle chassis ground are at different potentials — a classic ground loop.

**Identified risk:** The CD40 USB radio's audio input ground may be **floating** (not connected to vehicle chassis ground). Measure with a multimeter between the AUX input GND and vehicle chassis: if you read any voltage, the ground is floating. ([Issue #3, PNKP237](https://github.com/PNKP237/EHU32/issues/3#issuecomment-2094406036))

**Fix:** Use an **audio isolation transformer** (inline stereo ground loop isolator, ~€5–10) between the PCM5102A output and the radio's AUX input. See [aux-audio-connection.md](aux-audio-connection.md) for details.

---

### Noisy audio / interference from power supply

**Symptom:** High-frequency noise, whine, or buzzing that changes with engine RPM.

**Root cause:** Switch-mode buck converters and car USB chargers can inject switching noise into the audio path.

**Fix:**
1. Add a **100 nF ceramic capacitor** and a **220–1000 µF electrolytic capacitor** in parallel across the ESP32's `VIN` and `GND` pins. Place them as close to the pins as possible.
2. Linear regulators (e.g. 7805) produce less switching noise than buck converters but get warm — ensure adequate ventilation.

([Issue #1, PNKP237](https://github.com/PNKP237/EHU32/issues/1#issuecomment-2049820474))

---

## Display Issues

### Text not fitting on BID display

**Symptom:** Status messages like "Bluetooth connected" or "Coolant temp: XX°C" appear truncated on the BID (Basic Information Display).

**Root cause:** The BID has a shorter line width than GID or CID. EHU32 initially used strings optimised for GID/CID. Additionally, EHU32 did not originally know which display type was connected and could not choose shorter strings automatically. ([Issue #15](https://github.com/PNKP237/EHU32/issues/15))

**Status:** Fixed in EHU32 v0.9.5 and later. The display detection logic was improved and shorter BID-specific strings were added.

**Workaround for older firmware:** Flash a BID-optimised binary, available as an attachment in [Issue #15](https://github.com/PNKP237/EHU32/issues/15).

---

### Display shows garbled or truncated text for non-ASCII characters

**Symptom:** Song titles or status strings with accented characters (ü, ő, é, etc.) display incorrectly or cause text to be cut off at the first unrecognised character.

**Root cause:** Opel displays use UTF-16 encoding. EHU32's internal ASCII-to-UTF16 conversion did not support all extended characters in earlier versions. Characters with "double acute" (ő, ű) are not supported by some older GID variants (e.g. 2006 Vectra GID). ([Issue #1, PNKP237](https://github.com/PNKP237/EHU32/issues/1#issuecomment-1980799388))

**Fix:** Update to the latest firmware version; extended character support was improved in v0.7+.

---

### "ÿ" character appearing at end of display line

**Symptom:** A `ÿ` character appears at the end of the third line on a CID display when the title/artist/album text is very long.

**Root cause:** Buffer overflow in the display string — when the first two lines are completely filled, the third line's null terminator gets pushed into the visible buffer. ([Issue #6, xddxd](https://github.com/PNKP237/EHU32/issues/6#issuecomment-3086823618))

**Status:** Noted as a known issue; planned fix is to extend the buffer to support up to 512 characters.

---

### Display "freezes" when switching between audio metadata and vehicle data

**Symptom:** After switching from vehicle data mode (coolant, speed) back to audio metadata mode, the display keeps showing the previous data instead of updating to show the song title.

**Root cause:** EHU32 retransmits the last message to overwrite the radio's "Aux" display text. When switching modes, there is a brief period where no new data has arrived and the old data persists. ([Issue #15, PNKP237](https://github.com/PNKP237/EHU32/issues/15))

**Fix:** This was addressed in v0.9.5+ by adding a Bluetooth status placeholder shown when no audio metadata has yet been received.

---

### Head unit makes a "click" and restarts when using speed display mode

**Symptom:** After approximately 30 minutes in speed display mode (sending data every ~300ms), the head unit emits a click and restarts. This does not happen in coolant or audio metadata modes.

**Root cause:** Displays are not designed to receive a new message every 300ms — the radio normally resends data every 5 seconds. The high message rate can overwhelm the display/radio. ([Issue #15, kotucocuk](https://github.com/PNKP237/EHU32/issues/15))

**Status:** Under investigation by the developer. Planned fix: increase the interval between data frames and add forced display re-initialisation.

---

### Audio settings (bass, treble, balance) cannot be adjusted when EHU32 is connected

**Symptom:** After EHU32 activates, the radio's audio settings menu buttons become unresponsive on the BID display.

**Root cause:** EHU32 continuously overwrites display messages to show audio metadata. Some radios (particularly CD30MP3 + BID) send the audio settings menu via the same CAN messages, which EHU32 intercepts and suppresses. ([Issue #15, kotucocuk](https://github.com/PNKP237/EHU32/issues/15))

**Fix:** Hold the **"9" button for ~1 second** to temporarily disable EHU32's display writes. "Aux" will appear on screen, and you can navigate to the audio settings menu normally. EHU32 should detect text such as "Fader", "Balance", "Bass", "Treble", or "Sound off" and automatically stop blocking.

---

## Power Supply Issues

### CD40 USB port crashes the CAN bus

**Symptom:** When the ESP32 is powered from the CD40's USB port, the radio crashes or becomes unresponsive, and the CAN bus stops working.

**Root cause:** The CD40's USB port shares a common ground or power path with the CAN bus circuitry. Connecting the ESP32 via USB introduces enough noise or current draw to destabilise the bus. ([Issue #3, Vortecsz](https://github.com/PNKP237/EHU32/issues/3))

**Fix:** Never power the ESP32 from the CD40's USB port. Use a separate 12V→5V USB car charger, a buck converter from switched 12V, or power derived from the radio PCB's internal 5V rail.

---

### ESP32 goes to deep sleep immediately after power-on

**Symptom:** EHU32 powers on briefly, then immediately enters deep sleep. Bluetooth never activates.

**Root cause:** EHU32 checks for CAN bus activity within 100ms of power-up. If the radio is not already on and sending CAN messages, EHU32 assumes the vehicle is off.

**Fix:** Ensure the radio is powered on **before** or **at the same time as** EHU32. If using OBD-II for power (which is unswitched), this is normal behaviour — EHU32 will wake again the next time it detects CAN bus activity.

---

## Bluetooth Connection Issues

### iPhone audio issues (crackling, dropouts)

See [Crackling, stuttering, or audio dropouts](#crackling-stuttering-or-audio-dropouts) above. The root cause is the ESP32 board, not the iPhone model or iOS version.

---

### Huawei and some Android phones: poor audio quality

**Root cause:** Same hardware issue as iPhone stuttering — clone ESP32 boards with BoyaMicro flash chips cause problems with certain devices. ([Issue #6](https://github.com/PNKP237/EHU32/issues/6))

**Fix:** Replace the ESP32 module with a genuine Espressif one.

---

### EHU32 not visible in Bluetooth scan

**Symptom:** EHU32's Bluetooth device does not appear in the phone's Bluetooth scan.

**Root cause:** EHU32 only enables Bluetooth **after detecting CAN bus activity from the radio**. Without a running radio sending CAN messages, Bluetooth stays off.

**Fix:**
1. Make sure the head unit is switched on.
2. Verify CAN-H and CAN-L wiring.
3. Wait a few seconds for the radio to begin sending CAN messages.

---

## ESP32 Board Compatibility

### Confirmed problematic boards

- **ESP32-WROOM-32E clones** with a **BoyaMicro** flash chip (visible after removing the RF shield). These cause audio stuttering specifically with iPhones and some Android devices. ([Issue #6, PNKP237](https://github.com/PNKP237/EHU32/issues/6#issuecomment-3190095186))
- **ESP-32S boards** (first-generation module, no antenna connector): reported as generally unreliable. ([Issue #6, PNKP237](https://github.com/PNKP237/EHU32/issues/6#issuecomment-3726338115))
- **Boards from unknown AliExpress sellers** — quality is hit-or-miss; some work fine, some do not.

### Confirmed working boards

- **WeAct Studio ESP32 D0WD v3** — confirmed working with both iOS and Android by the developer. ([Issue #6, PNKP237](https://github.com/PNKP237/EHU32/issues/6))
- **Generic ESP-WROOM-32 / 32D / 32UE boards** with genuine Espressif RF module — confirmed working in testing by the developer.

### How to identify a genuine Espressif module

Look for **"Espressif"** etched (laser-engraved) into the metal RF shield on the top of the module. Fake modules often have nothing engraved, or have text that is printed rather than engraved. When in doubt, desolder the shield and look at the flash chip — BoyaMicro chips are a red flag.

See [community-knowledge.md](community-knowledge.md) for more detail.

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [Power Supply](power-supply.md)
- [AUX Audio Connection](aux-audio-connection.md)
- [Troubleshooting](troubleshooting.md)
- [Community Knowledge](community-knowledge.md)
