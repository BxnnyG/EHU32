# Community Knowledge Base

> This document was compiled from community discussions in the GitHub Issues. See the original threads for full context.
> Original issues: [#1](https://github.com/PNKP237/EHU32/issues/1) · [#3](https://github.com/PNKP237/EHU32/issues/3) · [#6](https://github.com/PNKP237/EHU32/issues/6) · [#15](https://github.com/PNKP237/EHU32/issues/15) · [#16](https://github.com/PNKP237/EHU32/issues/16)

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## ESP32 Board Compatibility

### Confirmed working boards

| Board | Notes |
|-------|-------|
| WeAct Studio ESP32 D0WD v3 | Recommended by developer; confirmed good with iOS and Android |
| ESP32-WROOM-32 (genuine Espressif) | Confirmed working |
| ESP32-WROOM-32D (genuine Espressif) | Confirmed working |
| ESP32-WROOM-32UE (genuine Espressif) | Confirmed working; external antenna variant |
| Generic board with IPX/U.FL antenna connector + genuine Espressif module | Generally good |

### Confirmed problematic boards

| Board | Problem |
|-------|---------|
| ESP32-WROOM-32E clones with BoyaMicro flash | Audio stuttering on iPhone and some Android phones |
| ESP-32S (first generation, PCB antenna only) | Generally unreliable; avoid |
| Unknown AliExpress boards (no Espressif branding) | Hit-or-miss quality |

### How to identify a genuine Espressif module

1. **Check the RF shield:** A genuine Espressif module has **"Espressif"** laser-engraved into the metal can. Fake modules often have nothing engraved, or have text printed on rather than cut in.
2. **Remove the shield:** If you're unsure, desolder the RF shield and inspect the flash memory chip. **BoyaMicro** flash chips are associated with problematic clone modules.
3. **Prefer modules with an external antenna connector (IPX/U.FL):** Boards with only a PCB (trace) antenna are generally less reliable for in-car use where the ESP32 may end up inside a steel radio enclosure.

([Issue #6](https://github.com/PNKP237/EHU32/issues/6))

---

## CD40 USB — Specific Warnings

The CD40 USB radio has several important differences compared to the CD30 and CD70:

### Do NOT power ESP32 from the CD40's USB port

Connecting the ESP32 to the CD40's USB port for power causes the CAN bus to crash, making the radio unresponsive. The exact cause is unclear but may relate to ground shared between the USB port and the CAN bus circuitry.

**Always use an external power source:** a 12V→5V USB car charger, a buck converter from switched 12V, or internal 5V from the PCB. ([Issue #3, Vortecsz & PNKP237](https://github.com/PNKP237/EHU32/issues/3))

### Floating audio ground on CD40

The CD40's audio input ground may be electrically floating — not connected to vehicle chassis ground. Connecting a PCM5102A or other audio device to a floating ground can cause audible hum/buzz (a ground loop).

**How to check:** Use a multimeter set to DC voltage between the AUX input GND and vehicle chassis (any bare metal bodywork). If you measure any voltage, the ground is floating.

**Fix:** Insert an **audio isolation transformer** (ground loop isolator) between the PCM5102A output and the CD40 AUX input. These are inexpensive (€5–10) and available at most electronics retailers. ([Issue #3, PNKP237](https://github.com/PNKP237/EHU32/issues/3))

---

## Display Character Limits and Formatting

Opel displays use **UTF-16** encoding for text. EHU32 handles the conversion from UTF-8, but with limitations.

### Character support

| Character set | Support |
|--------------|---------|
| ASCII (0–127) | Full |
| Extended Latin (128–255) | Mostly supported from firmware v0.7+ |
| Double-acute characters (ő, ű, etc.) | Not supported on some older GID variants (e.g. 2006 Vectra GID) |

If a string contains an unsupported character, display output is **cut off at the first unrecognised character**. For example, a song titled `áéíóöőúüű` would only show `áéíóö` if `ő` is not supported by the display firmware.

([Issue #1, PNKP237](https://github.com/PNKP237/EHU32/issues/1))

### Display types and layout

| Display | Lines for EHU32 | Notes |
|---------|----------------|-------|
| CID (Colour Information Display) | 3 | Widest; full metadata (artist, title, album). Bug: very long strings can produce trailing `ÿ` character |
| GID three-line | 3 | Standard three-line view; artist, title, album |
| GID single-line | 1 | Only one line visible; album/artist lines are hidden. Notifications appear in top-right corner |
| BID (Basic Information Display) | 1–2 | Shorter line width than GID/CID; requires shorter status strings. Confirmed working with CD30MP3 |
| TID (Trip Information Display) | 1–2 | Similar constraints to BID |

**Important:** EHU32 cannot dynamically detect which display type is connected at runtime (early firmware versions). Later versions (v0.9.5+) include improved display-type detection logic.

For BID specifically: EHU32 requires a **longer delay between data frames** to avoid triggering radio instability. This was identified through community testing and fixed in v0.9.5+. ([Issue #15](https://github.com/PNKP237/EHU32/issues/15))

### Customising display strings

To translate or modify the status strings (e.g. "Bluetooth connected", "Voltage:", "Coolant temp:"), edit the source file and recompile:

- Status messages are defined in **`CAN.ino`** (specific line numbers vary by version; search for the string literals).
- Accented characters can be used but should be tested — not all character codes display correctly on all display hardware.
- Avoid characters outside UTF-8 range 0–255 in earlier firmware versions.

([Issue #1, PNKP237](https://github.com/PNKP237/EHU32/issues/1))

---

## What Diagnostic Data Is Available

EHU32 connects to the **MS-CAN** bus (Medium Speed CAN, 95 kbit/s). This limits what data can be read.

### Available on MS-CAN (supported by EHU32)

| Data | Source module | Notes |
|------|--------------|-------|
| Bluetooth audio metadata | — | Artist, track title, album |
| Vehicle speed | Display / ECC | Read from display measurement blocks |
| Coolant temperature | Display / ECC | Requires Electronic Climate Control (ECC) for full accuracy; without ECC only display measurement blocks are used |
| Battery voltage | ECC | Requires Electronic Climate Control module |
| Engine RPM | ECC | Requires Electronic Climate Control module |

**Note:** Voltage, RPM, and accurate coolant temperature all require the car to be equipped with **Electronic Climate Control (ECC)** — the version with three knobs and an AC menu on the GID/CID. Manual climate control cars will only receive audio metadata and basic coolant data from the display.

### NOT available on MS-CAN (cannot be added to EHU32 without extra hardware)

| Data | Reason |
|------|--------|
| DPF saturation / diesel filter status | Only on HS-CAN (500 kbit/s) |
| Engine diagnostics (fault codes, etc.) | Only on HS-CAN |
| Fuel trim, injection data | Only on HS-CAN |

HS-CAN support would require an additional CAN module such as an MCP2515, because the ESP32 only has one built-in TWAI/CAN controller. This is outside the current scope of EHU32. ([Issue #1, PNKP237](https://github.com/PNKP237/EHU32/issues/1))

---

## Tips for Android and iOS Devices

### Android

- **Disable "Keep volume consistent":** Go to Bluetooth settings → tap on EHU32 → turn off "Keep volume consistent". This prevents Android from adjusting the audio stream volume locally, which causes clipping and reduced quality.
- **Set phone volume to maximum** for the best audio quality. EHU32 does not apply software volume reduction.
- Audio codec: Android connects using **SBC at 48 kHz** by default.

### iOS (iPhone)

- EHU32 has been confirmed working with iPhones. However, certain clone ESP32 boards cause audio stuttering — see [known-issues-and-solutions.md](known-issues-and-solutions.md).
- Muting the iPhone will not stop audio playback through EHU32 (by design — the receiver ignores remote volume control).
- If audio is stuttering, the fix is almost always the ESP32 board, not the phone or firmware.

([Issue #6](https://github.com/PNKP237/EHU32/issues/6))

---

## Power Supply Best Practices

1. **Use switched 12V (not always-on):** Power from a switched source (one that goes off with the ignition) prevents EHU32 from draining the battery. If using the OBD-II port for power, note that it provides **unswitched 12V** — EHU32 will repeatedly wake and sleep, consuming a small but non-zero current.
2. **Add filter capacitors:** Solder a **100 nF ceramic capacitor** and a **220–1000 µF electrolytic capacitor** across `VIN` and `GND` on the ESP32. This filters power supply noise that can otherwise contaminate the audio output.
3. **Do not power simultaneously from VIN and USB:** If a 5V source is connected to the `VIN` pin, do not also connect the ESP32 to a PC via USB at the same time — this can damage the board.
4. **Linear regulators produce cleaner audio:** If installing a regulator inside the radio enclosure, a linear regulator (e.g. 7805) produces less switching noise than a buck converter, at the cost of heat dissipation.

([Issue #1](https://github.com/PNKP237/EHU32/issues/1), [Issue #3](https://github.com/PNKP237/EHU32/issues/3))

---

## CAN Transceiver Options

The ESP32 needs an external CAN transceiver. The following have been confirmed compatible:

| Module | Supply voltage | Notes |
|--------|---------------|-------|
| MCP2551 | 5V | Mentioned in README; common |
| TJA1050 | 5V | Confirmed working |
| TJA1051 | 5V | Confirmed working |
| SN65HVD230 | 3.3V | Use this if your board's 5V supply is unreliable |

**Note on voltage:** MCP2551, TJA1050, and TJA1051 require a **5V** supply. SN65HVD230 runs on **3.3V**, which can be convenient since the ESP32's GPIO pins are 3.3V logic — no level shifting needed.

([Issue #1, PNKP237](https://github.com/PNKP237/EHU32/issues/1))

---

## OBD-II CAN Bus Pin Mapping

| OBD-II Pin | Signal | Notes |
|-----------|--------|-------|
| 3 | MS-CAN High (CAN-H) | Use this for EHU32 |
| 11 | MS-CAN Low (CAN-L) | Use this for EHU32 |
| 4 | Chassis GND | Can be tied to pin 5 |
| 5 | Signal GND | Can be tied to pin 4 |
| 6 | HS-CAN High | **Not used by EHU32** |
| 14 | HS-CAN Low | **Not used by EHU32** |

**Common mistake:** Some wiring guides list OBD-II pins 6 and 14 as CAN. These are **HS-CAN** (500 kbit/s), not MS-CAN. EHU32 requires **MS-CAN on pins 3 and 11**. ([Issue #1, PNKP237](https://github.com/PNKP237/EHU32/issues/1))

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply](power-supply.md)
- [AUX Audio Connection](aux-audio-connection.md)
- [Known Issues and Solutions](known-issues-and-solutions.md)
- [Radio-Specific Notes](radio-specific-notes.md)
