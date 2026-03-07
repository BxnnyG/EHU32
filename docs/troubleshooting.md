# Troubleshooting

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## Compilation & Upload Issues

### "Sketch too big" / "text section exceeds available space"

**Symptom:** Arduino IDE reports something like:
```
Sketch uses 137% of program storage space.
text section exceeds available space in Flash.
```

**Fix:** Change the partition scheme in Arduino IDE:
- Tools → Partition Scheme → **Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)**

Also ensure **Core Debug Level** is set to **None** (Tools → Core Debug Level → None). This reduces binary size.

See [Compilation Guide](compilation-guide.md) for full Arduino IDE settings.

---

### OpenOCD / JTAG error after upload

**Symptom:** After a successful upload, the Arduino IDE console shows an OpenOCD or JTAG-related error.

**Fix:** Ignore it. This error is produced by the debug button/interface and does not indicate a problem with the uploaded firmware. The ESP32 has been flashed correctly.

---

## Runtime / Vehicle Issues

### Radio crashes or becomes unresponsive when ESP32 is connected

**Symptom:** Connecting the ESP32 causes the radio to reboot, freeze, or display unusual behaviour.

**Possible causes and fixes:**
1. **ESP32 is powered from the radio's USB port.** Do not do this. Use a separate power source. See [Power Supply Options](power-supply.md).
2. **CAN bus wiring fault.** Check that CAN-H and CAN-L are not swapped, and that the transceiver VCC matches the module's requirement (5V or 3.3V). See [CAN Bus Connection](can-bus-connection.md).
3. **Incorrect CAN bus speed.** Verify that the sdkconfig TWAI settings have been applied. See [Compilation Guide](compilation-guide.md).

---

### No Bluetooth device visible / cannot pair

**Symptom:** EHU32 does not appear in your phone's Bluetooth scan.

**Explanation:** EHU32 only enables Bluetooth **after it detects CAN bus activity from the radio**. The radio must be powered on and communicating with the display over the MS-CAN bus before Bluetooth is activated.

**Fix:**
1. Make sure the head unit is switched on (engine or ignition is not required, but the radio must be on).
2. Wait for the radio to fully start up (it takes a few seconds to start sending CAN messages).
3. EHU32 should appear in the Bluetooth scan within a few seconds of detecting CAN activity.

---

### Audio crackling, stuttering, or dropouts

**Possible causes and fixes:**
1. **Android "Keep volume consistent" setting.** Go to Bluetooth settings on Android, tap on EHU32, and **disable "Keep volume consistent"**. This setting adjusts the Bluetooth stream volume repeatedly and causes stuttering.
2. **Non-Espressif ESP32 module.** Some ESP32 boards from third-party manufacturers have I2S timing issues, particularly with iPhones and Huawei phones. Use a board with an **Espressif** RF shield (look for "Espressif" etched on the metal can). See [Hardware Overview](hardware-overview.md).
3. **Noisy power supply.** Add a 100 µF electrolytic capacitor across ESP32 `VIN` and `GND`. See [Power Supply Options](power-supply.md).

---

### Display shows nothing / no startup message

**Symptom:** After powering on, the display remains blank or continues to show the radio's normal content with no EHU32 status message.

**Explanation:** On the **very first boot** (or after a factory reset), EHU32 runs a setup routine that tests the display and vehicle modules. This can take **30–40 seconds**. Be patient.

**Fix:**
- Turn the head unit on and wait up to 40 seconds.
- Make sure the radio is fully powered (not just in standby).
- If nothing appears after 40 seconds, check CAN bus wiring.

---

### ESP32 goes to deep sleep immediately

**Symptom:** EHU32 powers on briefly then immediately goes into deep sleep, and the Bluetooth device never appears.

**Explanation:** EHU32 checks for CAN bus activity on startup. If no activity is detected within 100ms, it assumes the vehicle is off and enters deep sleep to save power.

**Fix:** The radio must be powered on **before** EHU32 powers up, or at minimum simultaneously. If using the OBD-II port for power (which is unswitched), the ESP32 may wake briefly when the vehicle is off and find no CAN activity. This is normal behaviour — EHU32 will wake again when CAN bus activity resumes (i.e. when the radio is switched on).

---

### CAN bus errors / radio unresponsive after some time

**Symptom:** Initially works, but after a while the radio becomes unresponsive or EHU32 stops functioning.

**Fix:**
1. Verify sdkconfig TWAI modifications have been applied — see [Compilation Guide](compilation-guide.md).
2. Check CAN transceiver wiring, particularly the VCC supply.
3. Confirm the bus speed is configured for **95 kbit/s** (not 500 kbit/s).

---

### Wi-Fi hotspot "EHU32-OTA" appearing unexpectedly

**Symptom:** The EHU32 OTA Wi-Fi hotspot appears without you intentionally activating it.

**Explanation:** The OTA hotspot is only activated by holding button **"8"** on the radio for approximately 1 second. If it activates unexpectedly, button "8" may be stuck or being pressed by something.

**Fix:** Press the **"8"** button briefly to exit OTA mode, or perform a reset.

---

### Settings or display issues / want to start fresh

**Symptom:** Display shows wrong data, wrong mode, or you want to re-run the initial setup.

**Fix:** Hold button **"9"** for a total of **5 seconds**. This performs a factory reset of all EHU32 settings stored in flash, and the ESP32 will restart and run the full setup routine on next boot.

Note: Pressing "9" briefly (less than 5 seconds) only toggles the display update feature — hold it the full 5 seconds to reset.

---

## Related Pages

- [Compilation Guide](compilation-guide.md)
- [CAN Bus Connection](can-bus-connection.md)
- [Power Supply Options](power-supply.md)
- [FAQ](faq.md)
