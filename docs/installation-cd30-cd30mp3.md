# CD30 / CD30 MP3 Installation Guide

[← Documentation Index](README.md) | [← Main README](../README.md)

---

| Property | Value |
|----------|-------|
| **Manufacturer** | Delphi / Grundig |
| **Vehicles** | Astra H, Corsa D, Vectra C, Zafira B, Meriva A |
| **AUX input** | Internally available on PCB |
| **CAN bus** | MS-CAN (95 kbit/s) |

---

## Overview

The CD30 and CD30 MP3 are among the most common head units found in this vehicle family. The CD30 MP3 is the variant with MP3 CD playback capability; both use the same Delphi/Grundig platform and the installation procedure is effectively identical.

For a community-contributed installation reference, see: [Issue #3 comment by PNKP237](https://github.com/PNKP237/EHU32/issues/3#issuecomment-2121866276)

> ⚠️ **IMPORTANT:** Do **NOT** power the ESP32 from the CD30's USB port (if equipped). This has been confirmed to crash the CAN bus. Use a separate power source — see [Power Supply Options](power-supply.md).

---

## 1. Opening the Unit

The CD30/CD30 MP3 is a standard DIN-size unit. Remove it from the dashboard using DIN removal keys, then undo the screws on the metal sleeve to access the PCB.

---

## 2. Power (5V)

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> The exact 5V test point location on the CD30/CD30 MP3 PCB has not yet been photographically confirmed. The guidance below describes the general method; please contribute photos if you have them.

**General approach — finding 5V on the PCB:**

1. Locate a voltage regulator IC on the PCB. Common parts are **7805**, **LM1117-5.0**, or a similar fixed 5 V linear regulator (usually a TO-220 or SOT-223 package with three pins/pads).
2. With the radio powered on, use a multimeter (DC voltage, black probe to chassis/Quadlock Pin A1 GND) to identify which pin of the regulator outputs 5 V.
3. The output pin is your 5 V tap point. Solder a wire here for ESP32 power.

**Workaround if no 5V tap is found:**

Use **Quadlock Block A, Pin 7** (switched 12 V) with a small DC-DC buck converter set to 5 V output. Connect the buck converter output to the ESP32 `VIN` pin. See [Power Supply Options](power-supply.md).

---

## 3. CAN Bus on PCB

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> The exact CAN transceiver IC location and solder pad positions on the CD30/CD30 MP3 PCB have not yet been photographically confirmed.

**General approach:**

1. Locate the CAN transceiver IC on the PCB — it is typically an 8-pin SMD package such as **TJA1040**, **TJA1041**, or **PCA82C250**. Search near the Quadlock connector area.
2. Use the IC datasheet to identify the CANH and CANL bus pins.
3. Alternatively, use a continuity tester to trace from **Quadlock Block A, Pin 3 (CAN-H)** and **Block A, Pin 8 (CAN-L)** through the PCB to the transceiver pads, then solder there.

**Workaround — OBD-II port:**

If you do not want to open the radio or cannot find the CAN traces, connect directly to the OBD-II diagnostic port under the dashboard:

| OBD-II Pin | Signal |
|-----------|--------|
| Pin 3 | MS-CAN High (CAN-H) |
| Pin 11 | MS-CAN Low (CAN-L) |

See [CAN Bus Connection](can-bus-connection.md) for full details.

---

## 4. AUX Audio Wiring

The CD30 MP3 has an internal AUX input routed through the Quadlock connector. Standard audio cable colours apply:

| Colour | Signal |
|--------|--------|
| Red | Right channel (+) |
| White | Left channel (+) |
| Black | Ground (GND) |

### Via Quadlock Block C (external connection)

| Quadlock Block C Pin | Signal |
|----------------------|--------|
| Pin 17 | Left channel |
| Pin 18 | Audio GND |
| Pin 19 | Right channel |

### Direct PCB solder points (internal install)

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> The exact PCB solder pad locations for the AUX lines have not been photographically confirmed.

**How to locate them:**

Use a continuity tester (multimeter in beep/continuity mode) to trace from the Quadlock Block C pins (17, 18, 19) through the PCB to convenient, accessible pads or through-holes. Solder thin wires there and route them to the PCM5102A output.

---

## 5. Quick Reference

| What | Where |
|------|-------|
| 5V power | Voltage regulator output on PCB — or Quadlock A7 (12V) + buck converter |
| CAN-H | PCB transceiver pad — or OBD-II Pin 3 |
| CAN-L | PCB transceiver pad — or OBD-II Pin 11 |
| AUX Left | Quadlock C17 / PCB pad (trace from C17) |
| AUX Right | Quadlock C19 / PCB pad (trace from C19) |
| AUX GND | Quadlock C18 / PCB pad (trace from C18) |

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply Options](power-supply.md)
- [AUX Audio Connection](aux-audio-connection.md)
- [Troubleshooting](troubleshooting.md)
