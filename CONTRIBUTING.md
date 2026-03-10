# Contributing to EHU32

Thank you for your interest in contributing to EHU32! This project is built and maintained by volunteers, and every contribution — whether it's code, documentation, hardware testing, or CAN bus research — makes it better for everyone.

For a project overview, see the [README](README.md).

---

## Table of Contents

- [How to Report Bugs](#how-to-report-bugs)
- [How to Request Features](#how-to-request-features)
- [How to Contribute Code](#how-to-contribute-code)
- [How to Contribute Documentation](#how-to-contribute-documentation)
- [How to Contribute Hardware Research](#how-to-contribute-hardware-research)
- [Code of Conduct](#code-of-conduct)

---

## How to Report Bugs

Before opening a new issue, please search the [existing issues](https://github.com/PNKP237/EHU32/issues) to see if the problem has already been reported.

When filing a bug report, please include as much of the following information as possible:

### Bug Report Template

```
**Hardware**
- ESP32 board:
- CAN transceiver:
- DAC module (e.g. PCM5102A):
- Any other relevant modules:

**Vehicle**
- Make / Model / Year:
- Radio type (e.g. CD30MP3, CD40USB, CDC40 Opera):
- Display type (e.g. CID, GID single-line, GID three-line, BID, TID):

**Firmware**
- EHU32 version or commit SHA:
- ESP32 Arduino Core version:

**Steps to Reproduce**
1.
2.
3.

**Expected behavior:**

**Actual behavior:**

**Serial debug output:**
(Enable by uncommenting `#define DEBUG` in `EHU32.ino`, then paste the output here)
```

Serial debug output is extremely helpful for diagnosing issues. To enable it, uncomment `#define DEBUG` near the top of `src/EHU32.ino` before flashing.

---

## How to Request Features

1. Check the [existing issues](https://github.com/PNKP237/EHU32/issues) and [discussions](https://github.com/PNKP237/EHU32/discussions) to see if someone has already requested the same feature.
2. Open a new issue and describe the use case — what problem would this feature solve, and for which vehicles or configurations?
3. If you are proposing support for a new radio or display model, include CAN message captures if you have them. Even partial captures are useful.

---

## How to Contribute Code

### 1. Fork and Branch

1. Fork the repository to your own GitHub account.
2. Create a feature branch from `main`:
   ```
   git checkout -b feature/your-feature-name
   ```

### 2. Development Environment

- **Arduino IDE** with **ESP32 Arduino Core v2.0.17**
  - Newer versions have been found to be less stable; please use exactly this version.
- **Libraries** (install via the Arduino Library Manager or manually):
  - [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) **v1.8.7**
  - [arduino-audio-tools](https://github.com/pschatzmann/arduino-audio-tools) **v1.1.1**
- **`sdkconfig` modification** — the TWAI driver requires specific settings to work correctly. See the [Compilation notes](README.md#compilation-notes) in the README for the exact changes needed and the file location.
- **Arduino IDE board settings:**
  - Events on Core **0**
  - Arduino on Core **1**
  - Partition Scheme: **Minimal SPIFFS**

### 3. Code Style Guidelines

- Follow the existing code style: C-style, compact formatting, minimal whitespace.
- Use descriptive variable names.
- Comment CAN message identifiers and data bytes with their meaning (what the ID represents, what each byte does).
- Use `#define` for pin numbers and CAN message identifiers rather than magic numbers.
- Before submitting, enable `#define DEBUG` in `src/EHU32.ino` and verify that your changes produce sensible debug output and do not introduce regressions.

### 4. Submit a Pull Request

1. Push your branch to your fork.
2. Open a Pull Request against the `main` branch of this repository.
3. Include a clear description of:
   - What the change does
   - Which vehicles or configurations it was tested with
   - Any known limitations

---

## How to Contribute Documentation

Documentation contributions are very welcome. You can improve:

- The `docs/` folder — installation guides, wiring notes, CAN bus message tables, and other reference material.
- Wiki pages on GitHub.
- CAN bus message documentation for radio models that are not yet fully documented.

**Hardware installation photos are extremely valuable.** If you have installed EHU32 inside a radio unit and have photos of the solder points (5V, CAN, AUX), please share them — either in an issue, a pull request, or the wiki.

---

## How to Contribute Hardware Research

You do not need to write code to make a valuable contribution. Hardware research helps the project support more vehicles and configurations:

- **CAN bus captures** from radios or vehicles that are not yet supported.
- **PCB photos** with identified solder points: 5V tap, CAN-H/CAN-L, AUX left/right/GND.
- **Testing reports** from different ESP32 boards, phones (especially iPhones and Huawei devices), and vehicle models — documenting what works and what does not.

The best place to share this kind of research is a GitHub issue, where it can be found and referenced by others.

---

## Code of Conduct

- Be respectful and constructive in all interactions.
- This is a hobby project maintained by volunteers in their spare time. Please be patient.
- Feedback and criticism are welcome as long as they are directed at the work, not the person.
