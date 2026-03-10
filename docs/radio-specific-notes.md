# Radio-Specific Notes

> This document was compiled from community discussions in the GitHub Issues. See the original threads for full context.
> Original issues: [#3](https://github.com/PNKP237/EHU32/issues/3) · [#15](https://github.com/PNKP237/EHU32/issues/15) · [#16](https://github.com/PNKP237/EHU32/issues/16) · [#19](https://github.com/PNKP237/EHU32/issues/19)

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## CD30 / CD30 MP3

**Manufacturer:** Delphi / Grundig
**Vehicles:** Astra H, Corsa D, Vectra C, Zafira B, Meriva A

### AUX activation

The CD30 and CD30 MP3 have an **internal AUX input** that can be soldered directly to the PCM5102A output. Wiring:

| Signal | Cable colour (typical) | Quadlock Block C pin |
|--------|----------------------|---------------------|
| Left channel | White | 17 |
| GND | Black | 18 |
| Right channel | Red | 19 |

### Internal installation reference

The developer (PNKP237) builds and tests EHU32 using a CD30 MP3 as their primary development radio. An internal installation is documented in [Issue #3](https://github.com/PNKP237/EHU32/issues/3#issuecomment-2121866276).

**Reference hardware:** Developer's unit is a Delphi-Grundig CD30MP3, GM part number **13289932**, manufactured 2008.

### BID display note

The CD30 MP3 is the confirmed working head unit when paired with a **BID (Basic Information Display)**. The BID-specific optimisations in EHU32 v0.9.5+ were developed and tested with a CD30MP3 + BID setup. ([Issue #15](https://github.com/PNKP237/EHU32/issues/15))

### Button direction variation

The left/right track skip button behaviour can differ between CD30 MP3 variants. If track skip works in reverse (left = next, right = previous), the head unit's GM part number differs from the reference unit. Contact the developer or raise an issue with your GM part number if buttons are reversed. ([Issue #15, PNKP237](https://github.com/PNKP237/EHU32/issues/15))

### PCB-level solder points

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> Exact solder points for 5V power, CAN-H/CAN-L, and AUX on the CD30/CD30MP3 PCB have not yet been photographically documented. The following workarounds are confirmed to work:
>
> - **Power:** 12V switched from Quadlock pin A7 → 12V-to-5V buck converter → ESP32 VIN pin
> - **CAN:** OBD-II port pin 3 (CAN-H) and pin 11 (CAN-L)
> - **AUX:** External AUX cable from PCM5102A to the Quadlock Block C connector (pins 17/18/19)
>
> To find the 5V rail on the PCB, use a multimeter on DC voltage: negative probe on chassis/metal shielding, positive probe on the output pins of voltage regulator ICs (typically 3-pin or 8-pin SMD near the power input). 5.0V indicates the correct rail.
>
> To trace CAN on the PCB, follow the traces from Quadlock Block C pins 2 (CAN-L) and 3 (CAN-H) to the CAN transceiver IC (look for an 8-pin SMD, e.g. TJA1040).

---

## CD40 USB

**Manufacturer:** Delphi
**Vehicles:** Astra H, Corsa D, Vectra C, Zafira B

### AUX

The CD40 USB has a **factory AUX input**. Connect the PCM5102A output directly to the Quadlock Block C AUX pins.

### ⚠️ Critical warnings

#### 1. Do NOT use the CD40's USB port for power

Powering the ESP32 from the CD40's built-in USB port causes the CAN bus to crash and makes the radio unresponsive. Always use an **external power source** (USB car charger, buck converter, or internal 5V rail). ([Issue #3](https://github.com/PNKP237/EHU32/issues/3))

#### 2. Floating audio ground

The CD40's audio input ground may be **floating** (not connected to vehicle chassis ground). Connecting a PCM5102A with a shared ground to a floating input can create a ground loop.

**Check:** Measure with a multimeter (DC voltage) between the AUX GND pin and any chassis ground point. Any non-zero reading means the ground is floating.

**Fix:** Use an **audio isolation transformer** (stereo ground loop isolator) between the PCM5102A output and the CD40 AUX input. ([Issue #3, PNKP237](https://github.com/PNKP237/EHU32/issues/3))

### PCB-level solder points

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> As with CD30, no photographically confirmed internal solder points exist for the CD40 USB. Use external Quadlock connector or OBD-II port for CAN.

---

## CD70 Navi

**Manufacturer:** Siemens / VDO
**Vehicles:** Astra H, Vectra C, Signum, Zafira B

### Installation guide

The CD70 Navi has the **most detailed installation documentation** available for EHU32. See the official wiki page:

👉 [Hardware modification — EHU32 installation in CD70 Navi](https://github.com/PNKP237/EHU32/wiki/Hardware-modification-%E2%80%90-EHU32-installation-in-CD70-Navi)

The wiki covers step-by-step internal installation with photos, including 5V power tap, CAN solder points, and AUX wiring directly on the PCB.

### Notes

- The CD ROM drive must be present for the display to work correctly.
- This is the recommended radio for first-time builders due to the detailed documentation.
- AUX is internally available and can be activated without hardware modification in some units.

---

## DVD90 Navi

**Manufacturer:** Siemens / VDO
**Vehicles:** Astra H, Vectra C, Signum

### Confirmed compatible with CD70 Navi tutorial

The DVD90 Navi is **hardware-identical to the CD70 Navi** in terms of the EHU32 installation (CAN wiring, 5V power, AUX connection). The CD70 Navi wiki tutorial applies directly to the DVD90 Navi. ([Issue #16, PNKP237](https://github.com/PNKP237/EHU32/issues/16#issuecomment-3317514103))

Community member **VolodyaBaranets** confirmed a successful DVD90 Navi installation using the CD70 Navi instructions in October 2025, with the same internal PCB solder points. ([Issue #16, VolodyaBaranets](https://github.com/PNKP237/EHU32/issues/16))

### CAN access point

CAN can be accessed on the **underside of the wire harness connector** inside the DVD90 — confirmed by the developer with a photo in [Issue #16](https://github.com/PNKP237/EHU32/issues/16#issuecomment-3317714349). Alternatively, use OBD-II pins 3 and 11.

### PCM5102A setup reminder

For DVD90 (and all radios) the PCM5102A module must have its jumpers set correctly:
- **H1L**: bridged
- **H2L**: bridged
- **H3L**: **NOT bridged** (controlled by GPIO 23 in firmware)
- **SCK**: short to GND (either via the jumper pad or a wire from SCK to GND)

Incorrect H3L bridging is the most common cause of no audio output. ([Issue #16, PNKP237](https://github.com/PNKP237/EHU32/issues/16))

### 3D-printed enclosure

Community member **Technolog83** built a 3D-printed enclosure for the EHU32 modules to protect them from short circuits against the radio PCB. This is highly recommended for internal installations. ([Issue #16, Technolog83](https://github.com/PNKP237/EHU32/issues/16#issuecomment-3380636099))

### Installation guide

Use the CD70 Navi wiki tutorial:

👉 [Hardware modification — EHU32 installation in CD70 Navi](https://github.com/PNKP237/EHU32/wiki/Hardware-modification-%E2%80%90-EHU32-installation-in-CD70-Navi)

See also [installation-dvd90-navi.md](installation-dvd90-navi.md) and [Issue #16](https://github.com/PNKP237/EHU32/issues/16) for ongoing discussion.

---

## CDC40 Opera (Blaupunkt / Delphi)

**Manufacturer:** Blaupunkt (Opera), also Delphi variant
**Vehicles:** Astra H, Vectra C, Zafira B

### Status

> ⚠️ **NOT YET IMPLEMENTED** — CDC40 Opera support is planned. See [Issue #19](https://github.com/PNKP237/EHU32/issues/19) for discussion. The developer has acquired a Blaupunkt CDC40 Opera for testing.

The CDC40 Opera provides the **best factory sound quality** of all Opel/Vauxhall head units from this era. Community testing has established the full CAN message protocol needed to enable the AUX input via DAB emulation.

### How the AUX activation works (DAB emulation)

The CDC40 Opera does not have a conventional AUX input. Instead, it supports an optional **DAB45 digital radio module**, which connects via MS-CAN. When the DAB module emulates presence and signals a radio station, the head unit activates an analogue audio input — which is the same audio path as the DAB module's output.

EHU32 can emulate the DAB45 module over CAN and activate this audio input, then route the PCM5102A Bluetooth audio through it. ([Issue #19, Technolog83](https://github.com/PNKP237/EHU32/issues/19#issuecomment-3707340115))

This approach also works on the **CDC40 Delphi** variant. An alternative method for the Delphi variant activates AUX via a software bug, but that approach is less reliable.

To select AUX/DAB mode on the head unit, use the same menu item as for DAB radio selection (in the same position as "AUX" on CD70 Navi).

### CAN messages for DAB emulation

The following messages must be sent by EHU32 to activate the CDC40 Opera's AUX input. All messages are on **MS-CAN** (95 kbit/s).

#### 1. DAB box presence heartbeat

Send this message **every 300–400 ms** continuously to emulate a connected DAB module:

```
Identifier: 0x50D
DLC: 8
Data: 0x1D 0x21 0x42 0x16 0x20 0x00 0x00 0x00
```

#### 2. Switch to DAB/AUX mode (three messages, sent once when activating)

Send all three messages **each time** the head unit is switched to DAB/AUX mode:

**Message 1 — DAB signal received:**
```
Identifier: 0x562
DLC: 8
Data: 0x03 0x01 0x01 0x01 0x00 0x00 0x00 0x00
```

**Message 2 — AUX OFF (sent before AUX ON):**
```
Identifier: 0x562
DLC: 8
Data: 0x05 0xE3 0x05 0x01 0x10 0x10 0x00 0x00
```

**Message 3 — AUX ON:**
```
Identifier: 0x562
DLC: 8
Data: 0x05 0xE3 0x05 0x01 0x0F 0x10 0x00 0x00
```

Source: community research originally published on the diagnostics.org.uk forum by user *salakala*, referenced in [Issue #19, xymetox](https://github.com/PNKP237/EHU32/issues/19).

### Implementation notes (for contributors)

The following items are needed to implement CDC40 Opera support in EHU32:

1. Add the 0x50D heartbeat message to the periodic CAN transmission loop (300–400 ms interval).
2. When EHU32 detects that Bluetooth is connected (or a specific button is pressed), send the three 0x562 activation messages.
3. Determine the trigger mechanism: the developer has proposed using the Bluetooth connected event or an otherwise unused button on the CDC40. ([Issue #19, PNKP237](https://github.com/PNKP237/EHU32/issues/19#issuecomment-3666448157))
4. Audio input wiring: physically disconnect the audio output cable from the DAB45 module's audio header and replace it with the PCM5102A output. The EHU32 CAN messages keep the head unit's input active.

If you wish to implement this, please fork the repository and open a pull request. The developer will review and merge when time permits. ([Issue #19, PNKP237](https://github.com/PNKP237/EHU32/issues/19#issuecomment-3865201474))

---

## Related Pages

- [Documentation Index](README.md)
- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [AUX Audio Connection](aux-audio-connection.md)
- [Community Knowledge](community-knowledge.md)
- [Known Issues and Solutions](known-issues-and-solutions.md)
- [Installation — CD30/CD30MP3](installation-cd30-cd30mp3.md)
- [Installation — CD40 USB](installation-cd40usb.md)
- [Installation — CD70 Navi](installation-cd70-navi.md)
- [Installation — DVD90 Navi](installation-dvd90-navi.md)
- [Installation — CDC40 Opera](installation-cdc40-opera.md)
