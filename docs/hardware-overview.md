# Hardware Overview — Components & Wiring

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## Bill of Materials

| Component | Notes |
|-----------|-------|
| **ESP32 module** | Must have an IPX/U.FL antenna connector. Look for boards with **Espressif** etched on the RF shield — third-party modules have been found to cause I2S audio issues, especially with iPhones and Huawei phones. |
| **IPX antenna** | Any 2.4 GHz IPX antenna works. Salvaged from a broken laptop is fine. |
| **PCM5102A DAC module** | Must have configurable solder jumpers on the underside. See [aux-audio-connection.md](aux-audio-connection.md) for jumper configuration. **H3L must NOT be bridged.** |
| **CAN transceiver** | See table below. Match VCC to your power rail. |

### CAN Transceiver Options

| Part | VCC | Notes |
|------|-----|-------|
| MCP2551 | 5 V | Common, widely available |
| TDA1040 / TDA1050 | 5 V | OEM-style, compact |
| SN65HVD230 / SN65HVD231 / SN65HVD232 | 3.3 V | Directly compatible with ESP32 logic levels; active-low standby pin (KF50BD variant) is driven by PCM_ENABLE — see below |

> **Note:** The SN65HVD23x series operates at 3.3 V and requires no level-shifting between the CAN transceiver and the ESP32. The `PCM_ENABLE` output (GPIO 27, active LOW) is also used to release the SN65HVD230 from standby mode.

---

## ESP32 Pin Assignments

These are extracted directly from the source code.

### CAN / TWAI (`src/CAN.ino`, line 29)

```cpp
TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL)
```

| Signal | GPIO |
|--------|------|
| CAN TX (to transceiver TX pin) | **GPIO 5** |
| CAN RX (from transceiver RX pin) | **GPIO 4** |

> GPIO 4 is also configured as the external wake-up source (`esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0)`), so the ESP32 wakes from deep sleep as soon as CAN bus activity is detected (falling edge on RX).

### PCM5102A Control (`src/EHU32.ino`, line 55)

```cpp
const int PCM_MUTE_CTL=23, PCM_ENABLE=27;
```

| Signal | GPIO | Description |
|--------|------|-------------|
| PCM_MUTE_CTL | **GPIO 23** | Controls PCM5102A soft-mute (HIGH = unmuted) |
| PCM_ENABLE | **GPIO 27** | Enables PCM5102A power / wakes SN65HVD230 from standby (active LOW) |

### I2S (PCM5102A audio data)

The I2S pins are the defaults provided by the ESP32-A2DP library. Refer to the [ESP32-A2DP documentation](https://github.com/pschatzmann/ESP32-A2DP) for exact pin numbers, as they can vary by library version.

---

## Wiring Diagram

See **[EHU32_wiring.pdf](../EHU32_wiring.pdf)** for the complete connection diagram between the ESP32, PCM5102A, and CAN transceiver.

---

## Related Pages

- [CAN Bus Connection](can-bus-connection.md) — where to tap the MS-CAN bus
- [Power Supply Options](power-supply.md) — how to power the ESP32
- [AUX Audio Connection](aux-audio-connection.md) — PCM5102A jumpers and output wiring
- [Compilation Guide](compilation-guide.md) — building and flashing the firmware
