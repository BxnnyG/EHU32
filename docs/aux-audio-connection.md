# AUX Audio Connection

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## PCM5102A DAC Module

The PCM5102A is a stereo I2S DAC used to convert the ESP32's digital audio output to an analogue line-level signal suitable for the radio's AUX input.

### Jumper Configuration

The PCM5102A module has solder jumpers on the underside that must be configured correctly. **Incorrect jumper settings will result in no audio or distorted audio.**

| Jumper | Required Setting | Description |
|--------|-----------------|-------------|
| **H3L** | **Open (NOT bridged)** | I2S audio format selection. Must be open for standard I2S left-justified format. Bridging this jumper causes incorrect audio format and silence or noise. |
| FLT | Open (default) | Filter roll-off. Open = slow roll-off (recommended). |
| DEMP | Open (default) | De-emphasis. Open = disabled. |
| XSMT | Bridged (if present) | Soft mute control. If this jumper selects between hardware and software mute, leave on software control (default). EHU32 controls mute via GPIO 23. |

> ⚠️ **H3L must NOT be bridged.** This is the most common misconfiguration. If you hear no audio or only noise, check this jumper first.

---

## Output Connection

The PCM5102A produces a **line-level analogue audio output** on its OUTL (Left) and OUTR (Right) pins, with a common AGND (audio ground).

| PCM5102A Pin | Signal | Cable Colour (convention) |
|-------------|--------|--------------------------|
| OUTR | Right channel (+) | Red |
| OUTL | Left channel (+) | White |
| AGND | Audio ground | Black |

**The output is intended for amplifier input or AUX socket only. Never connect headphones or speakers directly to the PCM5102A output.**

Connect these wires to the radio's AUX input — either via the Quadlock connector or internal PCB solder points depending on your installation method. See the individual radio installation guides for exact pin/pad locations.

---

## Cable Colour Conventions

These are standard audio cable colour codes used throughout the installation guides:

| Colour | Signal |
|--------|--------|
| Red | Right channel (+) |
| White | Left channel (+) |
| Black | Ground (GND) |

---

## Ground Loop Issues (Especially CD40)

If you hear a constant hum or buzz through the radio speakers when EHU32 is in AUX mode, this is typically caused by a ground loop — a voltage difference between the audio GND and the vehicle chassis ground.

**How to diagnose:**
1. Set your multimeter to AC voltage mode.
2. Probe between the audio GND wire and chassis ground.
3. If you read any AC voltage (even millivolts), a ground loop is present.

**Fix:** Insert an **audio isolation transformer** (also sold as a "car audio ground loop isolator") in series with the Left and Right signal lines between the PCM5102A output and the radio's AUX input. This breaks the electrical connection between the two grounds while allowing the audio signal to pass.

> This is particularly likely with the CD40 USB — see [installation-cd40usb.md](installation-cd40usb.md).

---

## Volume Settings

- Set your **phone's volume to maximum** before pairing. Reducing the phone's volume introduces digital noise at low levels. Use the radio's volume knob or steering wheel controls for day-to-day volume adjustment.
- **Android users:** Go to Bluetooth settings, find EHU32 in the list of paired devices, and **disable "Keep volume consistent"**. This setting can cause audio dropouts and crackling by repeatedly adjusting the Bluetooth stream volume.

---

## Related Pages

- [Hardware Overview](hardware-overview.md) — ESP32 mute/enable pin assignments
- [Power Supply Options](power-supply.md) — shared GND considerations
- Installation guides for radio-specific AUX wiring locations
