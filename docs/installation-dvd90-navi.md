# DVD90 Navi Installation Guide

[← Documentation Index](README.md) | [← Main README](../README.md)

---

| Property | Value |
|----------|-------|
| **Manufacturer** | Siemens / VDO |
| **Vehicles** | Astra H, Vectra C, Zafira B, Signum |
| **AUX input** | Internally available |
| **CAN bus** | MS-CAN (95 kbit/s) |

---

> ⚠️ **IN PROGRESS — see [Issue #16](https://github.com/PNKP237/EHU32/issues/16)**
>
> The DVD90 Navi installation is actively being discussed in Issue #16 (33+ comments). Details in this document will be updated as confirmed information becomes available. If you have a DVD90 Navi and can contribute photos or verified PCB measurements, please comment on Issue #16.

---

## Overview

The DVD90 Navi is a higher-end navigation unit that shares a similar Siemens/VDO platform architecture with the CD70 Navi. The general EHU32 installation approach (power tap from 5V rail, CAN transceiver access, internal AUX solder points) is expected to be similar, but the exact PCB locations differ.

For the CD70 Navi reference installation, see [installation-cd70-navi.md](installation-cd70-navi.md) and the [CD70 Navi wiki article](https://github.com/PNKP237/EHU32/wiki/Hardware-modification-%E2%80%90-EHU32-installation-in-CD70-Navi).

---

## Power (5V)

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> Not yet confirmed for DVD90 Navi. See [Issue #16](https://github.com/PNKP237/EHU32/issues/16) for community progress.

General guidance: locate the 5V voltage regulator IC (7805, LM1117-5.0 or similar) and tap its output pin. See [Power Supply Options](power-supply.md) for the buck converter workaround if no 5V tap is found.

---

## CAN Bus on PCB

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> CAN transceiver IC location on the DVD90 Navi PCB is not yet confirmed.

**Workaround — OBD-II port:**

| OBD-II Pin | Signal |
|-----------|--------|
| Pin 3 | MS-CAN High (CAN-H) |
| Pin 11 | MS-CAN Low (CAN-L) |

---

## AUX Audio Wiring

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER**
>
> Internal AUX solder point locations on the DVD90 PCB are not yet confirmed.

---

## Community Discussion

Active installation discussion is ongoing in [Issue #16](https://github.com/PNKP237/EHU32/issues/16). Please follow that thread for the latest findings and contribute your experience if you have a DVD90 Navi.

---

## Related Pages

- [CD70 Navi Installation](installation-cd70-navi.md) — closest reference platform
- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply Options](power-supply.md)
- [AUX Audio Connection](aux-audio-connection.md)
