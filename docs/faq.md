# Frequently Asked Questions

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## Compatibility

### Which vehicles are supported?

EHU32 works with Opel/Vauxhall vehicles equipped with a compatible head unit and an MS-CAN display:

- Astra H
- Corsa D
- Vectra C
- Zafira B
- Meriva A
- Signum

### Which radios are supported?

| Radio | Notes |
|-------|-------|
| CD30 | Supported |
| CD30 MP3 | Supported |
| CD40 USB | Supported |
| CDC40 Opera | In progress — see [Issue #19](https://github.com/PNKP237/EHU32/issues/19) |
| CD70 Navi | Supported (best documented) |
| DVD90 Navi | In progress — see [Issue #16](https://github.com/PNKP237/EHU32/issues/16) |

All supported radios require an AUX input (internal or external).

### Which displays work?

| Display | Description |
|---------|-------------|
| CID | Colour Information Display |
| GID (single-line) | Graphic Information Display, one text line |
| GID (three-line) | Graphic Information Display, three text lines |
| BID | Basic Information Display |
| TID | Trip Information Display |

---

## Setup & Installation

### Do I need OPCOM or any diagnostic tool to install EHU32?

No. EHU32 is plug-and-play. No ECU coding, OPCOM, or other diagnostic software is required. EHU32 automatically discovers the display type and vehicle configuration on first boot.

### Can I install EHU32 without opening the radio?

Yes. For a non-invasive installation:
- Connect to the **OBD-II diagnostic port** (Pin 3 = CAN-H, Pin 11 = CAN-L) for the CAN bus connection.
- Use an **AUX cable** from the PCM5102A output to the radio's AUX input (Quadlock connector or front AUX socket if available).
- Power the ESP32 from a **USB car charger** plugged into the 12V accessory socket.

This requires no disassembly of the radio. See [CAN Bus Connection](can-bus-connection.md) and [Power Supply Options](power-supply.md).

### Does EHU32 work with iPhone?

Yes, but compatibility depends on the ESP32 board used. Some ESP32 boards have been found to cause audio issues (crackling, dropouts) specifically with iPhones and Huawei phones. Use an ESP32 board with a genuine **Espressif** RF module (look for "Espressif" etched on the metal shield). See [Hardware Overview](hardware-overview.md).

---

## Features

### Can I see DPF (Diesel Particulate Filter) status or other HS-CAN data?

No. EHU32 only accesses the **MS-CAN** (Medium Speed CAN, 95 kbit/s) bus. DPF status, engine diagnostics, and most powertrain data are on the **HS-CAN** (High Speed CAN, 500 kbit/s) bus, which EHU32 does not connect to.

EHU32 can display:
- Bluetooth audio metadata (artist, track title, album)
- Vehicle speed (from MS-CAN)
- Coolant temperature (from MS-CAN)
- Battery voltage (from MS-CAN)

### Can I update EHU32 without removing it from the car?

Yes, EHU32 supports **over-the-air (OTA) updates**:

1. Hold button **"8"** on the radio for approximately 1 second.
2. Connect your computer to the Wi-Fi hotspot: **SSID: `EHU32-OTA`**, **Password: `ehu32updater`**
3. In Arduino IDE, select the network port and upload as normal.
4. Hold **"8"** for 5 seconds to exit OTA mode.

### What is EHU32's power consumption?

The ESP32 enters **deep sleep** when no CAN bus activity is detected (i.e. when the radio is switched off). In deep sleep the power consumption is negligible (a few µA), so battery drain is not a concern for switched power installations.

If powered from the OBD-II port (unswitched), the ESP32 will briefly wake every few seconds to check for CAN activity, then go back to sleep. This represents a very small but non-zero current draw. For long periods of non-use, consider a switched power source.

---

## Audio

### Why do I hear a hum or buzz?

This is usually a ground loop. Check [AUX Audio Connection](aux-audio-connection.md) for diagnosis and the audio isolation transformer fix.

### Why is the audio quality poor / distorted?

- Make sure your phone's volume is set to **maximum** — reducing the Bluetooth stream volume introduces quantisation noise.
- Android users: disable **"Keep volume consistent"** in Bluetooth settings for EHU32.
- Check that the PCM5102A jumper **H3L is NOT bridged**.

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply Options](power-supply.md)
- [Compilation Guide](compilation-guide.md)
- [Troubleshooting](troubleshooting.md)
