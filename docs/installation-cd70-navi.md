# CD70 Navi Installation Guide

[← Documentation Index](README.md) | [← Main README](../README.md)

---

| Property | Value |
|----------|-------|
| **Manufacturer** | Siemens / VDO |
| **Vehicles** | Astra H, Vectra C, Zafira B |
| **AUX input** | Internally available |
| **CAN bus** | MS-CAN (95 kbit/s) |

---

## Overview

The CD70 Navi is the best-documented head unit for EHU32 installation. A dedicated wiki article with photos and step-by-step instructions is available:

➡️ **[Hardware modification — EHU32 installation in CD70 Navi (Wiki)](https://github.com/PNKP237/EHU32/wiki/Hardware-modification-%E2%80%90-EHU32-installation-in-CD70-Navi)**

The wiki article covers:
- Disassembly of the CD70 unit
- Location of the 5V power tap on the PCB
- CAN-H and CAN-L solder points at the CAN transceiver
- AUX input solder points
- Reassembly and cable routing

---

## Important Notes

- **CD ROM drive must be present.** The CD70 Navi requires the internal CD-ROM drive to be installed for the display to operate correctly. Removing the drive disables the display interface.
- The CD70 Navi uses a Siemens/VDO platform which differs internally from the Delphi-based CD30/CD40 units. Do not attempt to cross-reference PCB locations between these platforms.

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply Options](power-supply.md)
- [AUX Audio Connection](aux-audio-connection.md)
- [Troubleshooting](troubleshooting.md)
