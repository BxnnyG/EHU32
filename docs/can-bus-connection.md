# CAN Bus Connection Details

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## MS-CAN Overview

EHU32 uses the **MS-CAN** (Medium Speed CAN) bus, sometimes called the **Body CAN** or **Comfort CAN** in Opel/Vauxhall vehicles. Key characteristics:

| Property | Value |
|----------|-------|
| Bus speed | **95 kbit/s** |
| Standard CAN speed | 500 kbit/s (NOT used here) |
| Protocol | ISO 11898 / TWAI |
| Termination | 120 Ω at each end (typically inside ECUs) |

> ⚠️ **The bus speed is 95 kbit/s, not the standard 500 kbit/s.** The TWAI driver must be configured with custom timing parameters — see [Compilation Guide](compilation-guide.md).

---

## Access Points

You can tap into the MS-CAN bus at several locations in the vehicle. Choose the one most convenient for your installation:

### OBD-II Diagnostic Port (easiest — no disassembly required)

Located under the dashboard on the driver's side. This is the most accessible tap point and does not require removing the radio.

| OBD-II Pin | Signal |
|-----------|--------|
| Pin 3 | MS-CAN High (CAN-H) |
| Pin 11 | MS-CAN Low (CAN-L) |

> **Note:** The OBD-II port is always live (unswitched 12V on Pin 16). If you power the ESP32 from this port, it will never go into deep sleep and will drain the battery. See [Power Supply Options](power-supply.md).

### Quadlock Connector (at the head unit)

Quadlock Block A carries the CAN bus signals. The exact pin assignment matches between all head units in this vehicle family:

| Quadlock Block A Pin | Signal |
|----------------------|--------|
| Pin 3 | MS-CAN High (CAN-H) |
| Pin 8 | MS-CAN Low (CAN-L) |

### Radio PCB — CAN Transceiver IC

For internal installations, you can solder directly to the CAN transceiver IC on the radio's PCB. The transceiver is typically an 8-pin SMD package (TJA1040, TJA1041, PCA82C250 or similar).

> ⚠️ **NEEDS PHOTOS/DETAILS FROM OWNER** for specific radio models. Refer to the individual installation guides for model-specific PCB locations.

### Other Access Points

The MS-CAN bus is also accessible at:

- **Display connector** (CID/GID/BID/TID) — two wires in the display harness
- **Climate control panel (ECC) connector** — if the vehicle has the automatic climate control
- **Factory Bluetooth hands-free module** (if equipped) — typically mounted in the dashboard or centre console

---

## Wiring to the ESP32

Connect the CAN transceiver module to the ESP32 as follows:

| CAN Transceiver Pin | ESP32 GPIO |
|--------------------|-----------|
| TX (transceiver input from MCU) | **GPIO 5** |
| RX (transceiver output to MCU) | **GPIO 4** |
| CANH | CAN bus CAN-H wire |
| CANL | CAN bus CAN-L wire |
| VCC | 5V or 3.3V — match to transceiver type (see [Hardware Overview](hardware-overview.md)) |
| GND | Common GND |

---

## CAN Transceiver Selection

| Transceiver | VCC | Notes |
|-------------|-----|-------|
| MCP2551 | 5 V | Widely available, proven, requires 5V supply |
| TDA1040 / TDA1050 | 5 V | Compact, OEM-style |
| SN65HVD230 / SN65HVD231 / SN65HVD232 | 3.3 V | Best match for ESP32; no level shifting needed |

---

## TWAI Driver Configuration

The standard ESP32 TWAI driver must be configured for 95 kbit/s using custom timing parameters. Additionally, the sdkconfig must be modified to ensure reliable operation.

See [Compilation Guide](compilation-guide.md) for the complete sdkconfig section and Arduino IDE settings.

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [Power Supply Options](power-supply.md) — OBD-II power warning
- [Compilation Guide](compilation-guide.md) — TWAI/sdkconfig settings
- Individual installation guides for radio-specific access points
