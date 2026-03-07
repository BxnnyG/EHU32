# Power Supply Options

[← Documentation Index](README.md) | [← Main README](../README.md)

---

## Overview

The ESP32 can be powered from its `VIN` pin (5V) or through its USB connector. **Never connect both at the same time** — this can damage the ESP32 or cause unpredictable behaviour.

---

## Option A: Direct 5V from Radio PCB (Recommended for Internal Install)

**How it works:** Tap the 5V output of the radio's internal voltage regulator IC.

**Pros:**
- Cleanest solution for an internal install — everything is self-contained inside the radio unit
- Power is switched with the radio (no battery drain when radio is off)

**Cons:**
- Requires identifying the 5V test point, which varies by radio model and is not confirmed for all models

**How to find 5V on the PCB:**
1. Locate a 3-pin voltage regulator IC (e.g. 7805, LM1117-5.0, or similar) on the PCB.
2. With the radio powered on, use a multimeter (DC voltage mode, black probe to chassis GND) to probe the IC pins.
3. The output pin reading ~5V is your tap point.
4. Solder a thin wire to the output pin, route to ESP32 `VIN`.

> ⚠️ Confirmed PCB tap points for specific radios — see the individual installation guides. Not all models have confirmed photos yet.

---

## Option B: 12V Switched (Quadlock) + Buck Converter

**How it works:** Take 12V from Quadlock Block A, Pin 7 (switched 12V, live only when ignition/radio is on) and step it down to 5V using a small DC-DC buck converter module.

**Pros:**
- Works on any radio, no PCB probing needed
- Power is switched — ESP32 powers off when the radio is off

**Cons:**
- Slightly more components (buck converter module)
- Requires routing a wire to the Quadlock connector

**Wiring:**
```
Quadlock A7 (12V switched) → Buck converter IN+
Quadlock A1 (GND)          → Buck converter IN−
Buck converter OUT+ (5V)   → ESP32 VIN
Buck converter OUT−        → ESP32 GND
```

Set the buck converter output to **5.0V** before connecting the ESP32.

**Recommended addition:** Solder a **100 µF electrolytic capacitor** across the ESP32 `VIN` and `GND` pins to smooth out voltage spikes and noise, especially if audio crackling is observed.

---

## Option C: USB Car Charger (Simplest)

**How it works:** Plug a standard 5V USB car charger into the 12V accessory socket. Connect a USB cable to the ESP32.

**Pros:**
- Simplest possible solution — no soldering required for power
- Works well for non-internal (external) installations

**Cons:**
- Takes up the accessory socket
- Power is unswitched if using the cigarette lighter socket (stays on after ignition off)

---

## Important Warnings

### Do NOT power from the radio's USB port

Several radio models (CD30, CD40 USB) have USB ports. **Do not use these to power the ESP32.** This has been confirmed to crash the CAN bus, causing the radio and display to malfunction. This is documented in [Issue #3](https://github.com/PNKP237/EHU32/issues/3).

### OBD-II port provides unswitched 12V

If you tap power from the OBD-II diagnostic port, be aware that **Pin 16 (12V) is always live** — it does not switch off with the ignition. The ESP32 will enter deep sleep when no CAN activity is detected (i.e. when the radio is off), but it will not fully power down. This will cause a small but continuous battery drain.

For long-term installations, use a switched power source (Option A or B above).

### Never connect VIN and USB simultaneously

The ESP32 has a diode on its USB power input, but applying 5V to both `VIN` and the USB connector at the same time is outside the design specification and may cause damage.

---

## ESP32 Power Pins Summary

| Pin | Voltage | Notes |
|-----|---------|-------|
| `VIN` | 5V | Main power input for all options above |
| `USB` | 5V via cable | Alternative; do not use simultaneously with VIN |
| `3.3V` | 3.3V output | Output only — do not use as power input |
| `GND` | 0V | Common ground — must share with audio GND |

---

## Related Pages

- [Hardware Overview](hardware-overview.md)
- [CAN Bus Connection](can-bus-connection.md) — OBD-II power caveat
- [AUX Audio Connection](aux-audio-connection.md) — grounding notes
- Installation guides for radio-specific 5V tap locations
