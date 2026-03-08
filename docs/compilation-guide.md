# Compilation Guide — Building & Flashing EHU32

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## Required Software & Library Versions

| Component | Required Version | Notes |
|-----------|-----------------|-------|
| Arduino IDE | Any recent version | Tested with Arduino IDE 2.x |
| **ESP32 Arduino Core** | **2.0.17** | Newer versions have been found to be less stable. Do not use 3.x. |
| **ESP32-A2DP library** | **v1.8.7** | By Phil Schatzmann (pschatzmann) |
| **arduino-audio-tools library** | **v1.1.1** | By Phil Schatzmann (pschatzmann) |

Library links:
- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)
- [arduino-audio-tools](https://github.com/pschatzmann/arduino-audio-tools)

---

## Arduino IDE Settings

Open **Tools** menu and set the following:

| Setting | Required Value |
|---------|---------------|
| Board | ESP32 Dev Module (or your specific board) |
| **Events Run On** | **Core 0** |
| **Arduino Runs On** | **Core 1** |
| **Partition Scheme** | **Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)** |
| Core Debug Level | None |

> ⚠️ **Partition Scheme is critical.** The default partition scheme does not allocate enough space for the EHU32 firmware. If you see **"Sketch uses X% of program storage space"** warnings over 100%, or the error **"text section exceeds available space in Flash"**, you have the wrong partition scheme selected. Switch to **Minimal SPIFFS** and recompile.
>
> Setting Core Debug Level to **None** reduces the binary size and is required to fit within the Minimal SPIFFS partition.

---

## sdkconfig Modification (TWAI Driver)

The ESP32 TWAI (CAN) driver has a known errata issue that must be corrected by modifying the `sdkconfig` file shipped with the ESP32 Arduino Core.

**File location (Windows):**
```
%USERPROFILE%\AppData\Local\Arduino15\packages\esp32\hardware\esp32\<version>\tools\sdk\esp32\sdkconfig
```

Replace `<version>` with the ESP32 core version you installed (e.g. `2.0.17`).

**Linux/macOS:** The file is typically at:
```
~/.arduino15/packages/esp32/hardware/esp32/<version>/tools/sdk/esp32/sdkconfig
```

**What to change:**

Find the `# TWAI configuration` section and make it look exactly like this:

```
#
# TWAI configuration
#
CONFIG_TWAI_ISR_IN_IRAM=y
CONFIG_TWAI_ERRATA_FIX_BUS_OFF_REC=y
CONFIG_TWAI_ERRATA_FIX_TX_INTR_LOST=n
CONFIG_TWAI_ERRATA_FIX_RX_FRAME_INVALID=y
CONFIG_TWAI_ERRATA_FIX_RX_FIFO_CORRUPT=y
# CONFIG_TWAI_ERRATA_FIX_LISTEN_ONLY_DOM is not set
# end of TWAI configuration
```

Key changes:
- `CONFIG_TWAI_ISR_IN_IRAM=y` — runs the TWAI interrupt service routine from IRAM for reliability
- `CONFIG_TWAI_ERRATA_FIX_TX_INTR_LOST=n` — **disables** a buggy errata fix that causes TX interrupt loss

After editing, save the file and restart Arduino IDE to pick up the changes.

---

## Compiling

1. Open the `src/` folder in Arduino IDE (open `EHU32.ino`; the IDE will include all `.ino` files in the same folder automatically).
2. Verify the board, partition scheme, and core assignment settings are correct (see above).
3. Click **Verify/Compile** (Ctrl+R). If compilation succeeds with no errors, proceed to upload.

**Common compile error:**
```
Sketch uses 137% of program storage space.
text section exceeds available space in Flash.
```
→ Wrong partition scheme. Select **Minimal SPIFFS** in Tools → Partition Scheme.

---

## Uploading (USB)

1. Connect the ESP32 to your computer via USB.
2. Select the correct **Port** in Tools → Port.
3. Click **Upload** (Ctrl+U).
4. After upload completes, you may see an OpenOCD/JTAG-related error message in the console. **This is normal and can be ignored** — it is triggered by the debug button, not the upload process.

---

## OTA (Over-the-Air) Updates

Once EHU32 is installed in the vehicle, you can update the firmware wirelessly:

1. Hold button **"8"** on the radio panel for approximately 1 second.
2. EHU32 enables a Wi-Fi access point: **SSID: `EHU32-OTA`**, **Password: `ehu32updater`**
3. Connect your computer to this Wi-Fi network.
4. In Arduino IDE, the ESP32 should appear as a network port under Tools → Port.
5. Upload as normal.
6. To exit OTA mode, hold **"8"** for 5 seconds.

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md) — TWAI timing configuration context
- [Troubleshooting](troubleshooting.md) — compilation and upload errors
