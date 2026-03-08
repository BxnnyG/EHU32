# CDC40 Opera Installation Guide

[← Documentation Index](README.md) | [← Main README](../README.md)

---

| Property | Value |
|----------|-------|
| **Manufacturer** | Blaupunkt (Opera) / Delphi |
| **AUX input** | Via DAB emulation (special CAN messages required) |
| **CAN bus** | MS-CAN (95 kbit/s) |

---

> ⚠️ **NOT YET IMPLEMENTED IN EHU32 — see [Issue #19](https://github.com/PNKP237/EHU32/issues/19)**
>
> CDC40 Opera support is under consideration. The required CAN messages have been identified (see below), but are not yet implemented in the EHU32 firmware. Follow [Issue #19](https://github.com/PNKP237/EHU32/issues/19) for progress.

---

## Overview

The CDC40 Opera (Blaupunkt variant) does not have a conventional AUX input that can be enabled by wiring alone. Instead, AUX mode is activated by emulating a DAB (Digital Audio Broadcasting) tuner box over CAN bus. The radio switches to DAB/AUX mode in response to specific CAN messages.

This technique was originally discovered by *salakala* on the diagnostics.org.uk forum. It also works on the CDC40 Delphi variant. On the CDC40 Opera Delphi, AUX can alternatively be enabled through a software/configuration method (not requiring DAB emulation).

---

## Required CAN Messages

Once implemented, EHU32 would need to send the following messages to enable AUX on the CDC40 Opera:

### DAB Box Presence (sent every 300–400 ms, always)

This message announces to the radio that a DAB tuner is present:

```
Msg_DAB_Box_present = {
    .identifier = 0x50D,
    .data_length_code = 8,
    .data = { 0x1D, 0x21, 0x42, 0x16, 0x20, 0x00, 0x00, 0x00 }
}
```

### DAB Signal / AUX Mode Messages (sent when switching to DAB/AUX mode)

These three messages are sent together when the radio switches to DAB mode to activate AUX audio:

```
Msg_DAB_signal_receive = {
    .identifier = 0x562,
    .data_length_code = 8,
    .data = { 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 }
}

Msg_CDC40BLAU_AUX_OFF = {
    .identifier = 0x562,
    .data_length_code = 8,
    .data = { 0x05, 0xE3, 0x05, 0x01, 0x10, 0x10, 0x00, 0x00 }
}

Msg_CDC40BLAU_AUX_ON = {
    .identifier = 0x562,
    .data_length_code = 8,
    .data = { 0x05, 0xE3, 0x05, 0x01, 0x0F, 0x10, 0x00, 0x00 }
}
```

Source: [Issue #19](https://github.com/PNKP237/EHU32/issues/19), originally from the diagnostics.org.uk forum.

---

## Hardware Installation

Since this is not yet implemented, hardware-specific installation details have not been confirmed. The physical wiring (power, CAN bus, audio output) would follow the same principles as other Delphi-platform units:

- **Power:** See [Power Supply Options](power-supply.md).
- **CAN Bus:** See [CAN Bus Connection](can-bus-connection.md).
- **Audio output:** The PCM5102A output connects to the radio's AUX input. Depending on the variant, this may be accessible via the Quadlock connector or internal PCB pads.

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply Options](power-supply.md)
- [AUX Audio Connection](aux-audio-connection.md)
- [Issue #19 — Add support CDC40 Opera](https://github.com/PNKP237/EHU32/issues/19)
