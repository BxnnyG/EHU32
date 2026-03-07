# CD40 USB Installation Guide

[← Documentation Index](README.md) | [← Main README](../README.md)

---

| Property | Value |
|----------|-------|
| **Manufacturer** | Delphi |
| **Vehicles** | Astra H, Corsa D, Vectra C, Zafira B |
| **AUX input** | Factory available |
| **CAN bus** | MS-CAN (95 kbit/s) |

---

## Overview

The CD40 USB is a Delphi unit with a front USB port. The AUX input is available from the factory via the Quadlock connector.

> ⚠️ **IMPORTANT — Read before wiring:**
>
> - **Do NOT power the ESP32 from the CD40's USB port.** Powering the ESP32 via the radio's USB port causes a CAN bus crash, rendering the radio and display unresponsive. This has been documented in [Issue #3](https://github.com/PNKP237/EHU32/issues/3). Use a separate, isolated power source instead — see [Power Supply Options](power-supply.md).
> - **Audio ground may be floating.** Before connecting the PCM5102A audio output directly to the CD40's AUX input, check with a multimeter (AC voltage) between the audio GND wire and vehicle chassis ground. If you measure a voltage (even a small AC voltage), the audio ground is floating and connecting directly will introduce a hum/buzz. In this case, use an **audio isolation transformer** in series with the Left and Right signal lines.

---

## 1. Power Supply

Do not use the CD40's USB port for ESP32 power. Recommended options:

- **External USB car charger** plugged into the 12V accessory socket — simplest solution for a non-internal install.
- **Buck converter** from Quadlock Block A, Pin 7 (switched 12V ignition) → set output to 5V → connect to ESP32 `VIN`.

**Tip from the developer (from Issue #3 discussion):** Add a small **electrolytic capacitor** (e.g. 100 µF, 10V or higher) across the ESP32 `VIN` and `GND` pins. This smooths out power supply noise that can otherwise cause audio crackling or spurious resets.

For full power option details, see [Power Supply Options](power-supply.md).

---

## 2. CAN Bus

The MS-CAN bus is available via:

- **Quadlock Block A** — Pin 3 (CAN-H), Pin 8 (CAN-L)
- **OBD-II port** — Pin 3 (CAN-H), Pin 11 (CAN-L)

See [CAN Bus Connection](can-bus-connection.md) for full details on the bus and transceiver wiring.

---

## 3. AUX Audio Wiring

| Colour | Signal |
|--------|--------|
| Red | Right channel (+) |
| White | Left channel (+) |
| Black | Ground (GND) |

### Quadlock Block C

| Pin | Signal |
|-----|--------|
| 17 | Left channel |
| 18 | Audio GND |
| 19 | Right channel |

> If you detect any voltage between the audio GND and chassis GND with a multimeter, insert an **audio isolation transformer** between the PCM5102A output and the AUX input lines to eliminate ground loop hum.

---

## 4. Quick Reference

| What | Where |
|------|-------|
| 5V power | External USB charger, or Quadlock A7 (12V) + buck converter |
| CAN-H | Quadlock A3 — or OBD-II Pin 3 |
| CAN-L | Quadlock A8 — or OBD-II Pin 11 |
| AUX Left | Quadlock C17 |
| AUX Right | Quadlock C19 |
| AUX GND | Quadlock C18 |

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply Options](power-supply.md)
- [AUX Audio Connection](aux-audio-connection.md)
- [Troubleshooting](troubleshooting.md)
