# Startup Guide — Smart Irrigation System

**Goal**: Verify hardware step by step, starting with a single sensor before scaling up.
**Status legend**: [ ] Not started | [x] Done | [~] In progress | [!] Blocked

---

## Phase 1 — Environment Setup

- [ ] **1.1** Install VS Code
- [ ] **1.2** Install the PlatformIO extension in VS Code
  - Extensions panel → search "PlatformIO IDE" → Install
  - Wait for PlatformIO to finish installing its core (can take a few minutes, watch the bottom bar)
- [ ] **1.3** Open the project folder in VS Code
  - File → Open Folder → select `arduino-uno-aws-irrigation-system/`
  - PlatformIO should detect `platformio.ini` automatically
- [ ] **1.4** Let PlatformIO install the toolchain
  - First build triggers the download of `atmelavr` platform + AVR-GCC toolchain (~200 MB)
  - You only need to do this once

---

## Phase 2 — Single Sensor Test ✓ DONE

**Firmware**: `src/single_sensor_test.cpp` | **Env**: `single_sensor_test`

### Wiring

| Component | Arduino Pin |
|---|---|
| Moisture sensor — VCC | 5V |
| Moisture sensor — GND | GND |
| Moisture sensor — AOUT | A0 |

### Commands

```bash
platformio run -e single_sensor_test --target upload
platformio device monitor -e single_sensor_test
```

- [x] **2.1** Build and upload succeeded
- [x] **2.2** Values print continuously
- [x] **2.3** Value drops when sensor dipped in water, rises in dry air

### Calibration readings

| Condition | Reading |
|---|---|
| Fully dry (air) | _____ |
| Fully wet (water) | _____ |
| Threshold used | 450 (update `DRY_THRESHOLD` in the firmware if needed) |

---

## Phase 3 — Single Sensor + Relay/Pump (current step)

**Firmware**: `src/sensor_pump_test.cpp` | **Env**: `sensor_pump_test`

### Wiring (add to existing sensor wiring)

| Component | Arduino Pin | Notes |
|---|---|---|
| Relay IN1 | D2 | Signal from Arduino |
| Relay VCC | 5V | Power for the relay logic |
| Relay GND | GND | |
| Pump | Relay NO + COM | Wired through the relay's normally-open contacts |

**Active LOW relay**: D2 LOW = relay ON = pump ON. D2 HIGH = relay OFF = pump OFF.

**Power note**: if your pump draws more than ~250 mA, power it from a separate 5V supply — not the Arduino's USB 5V pin.

### Build and upload

```bash
platformio run -e sensor_pump_test --target upload
```

Or: **PROJECT TASKS → sensor_pump_test → Upload**

### Open the serial monitor at 9600 baud

```bash
platformio device monitor -e sensor_pump_test
```

Expected output:
```
=== Sensor + Pump Test ===
Pump starts OFF.
==========================
A0: 712  ->  DRY  |  Pump: ON
A0: 714  ->  DRY  |  Pump: ON
A0: 198  ->  WET  |  Pump: OFF
```

- [ ] **3.1** Build and upload succeeded
- [ ] **3.2** Pump state shown correctly in serial output
- [ ] **3.3** Relay **clicks ON** (audible click) when sensor reads above 450 (dry soil)
- [ ] **3.4** Relay **clicks OFF** when sensor is dipped in water
- [ ] **3.5** Actual pump runs when relay is ON (once pump is wired to relay)

---

## Phase 4 — Add Remaining Sensors

Once sensor 1 + relay are confirmed working, add sensors 2–4 one at a time.

| Sensor | Relay | Arduino pins |
|---|---|---|
| Sensor 2 | Relay IN2 | A1 + D3 |
| Sensor 3 | Relay IN3 | A2 + D4 |
| Sensor 4 | Relay IN4 | A3 + D5 |

- [ ] **4.1** Sensor 2 + relay 2 (A1, D3) verified
- [ ] **4.2** Sensor 3 + relay 3 (A2, D4) verified
- [ ] **4.3** Sensor 4 + relay 4 (A3, D5) verified
- [ ] **4.4** All 4 sensors + 4 relays wired and tested together

---

## Phase 5 — Add Extra Sensors

- [ ] **5.1** DHT22 (temperature + humidity) on D6 wired and reading correctly
  - Requires a 10 kΩ pull-up resistor between the data pin and 5V
- [ ] **5.2** LDR (light sensor) on A4 wired and reading correctly

---

## Phase 6 — Next Steps (Future)

Once all hardware is verified:

- [ ] **6.1** Add per-sensor thresholds (different plants need different moisture levels)
- [ ] **6.2** Add pump safety limits (30s max ON, 5min cooldown)
- [ ] **6.3** Integrate ESP32 for WiFi connectivity (`edge-ai/esp32-ml/`)
- [ ] **6.4** Enable the full ML pipeline (`src/main.cpp` + `lib/LocalMLEngine/`)

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Upload fails: "Port not found" | USB cable or driver issue | Try another USB cable; install CH340 driver if needed |
| Upload fails: "Permission denied" | Port locked by another app | Close Arduino IDE / other serial monitors |
| Reading is always 0 or 1023 | Sensor VCC/GND not connected | Check wiring at the sensor connector |
| Reading doesn't change in water | Wrong pin or floating analog input | Confirm signal wire is on A0, not a digital pin |
| Relay never clicks | Wrong pin / polarity | Verify IN1 is on D2; confirm relay is active-LOW type |
| Pump clicks but no water | Pump power supply | Run pump from separate power source, not Arduino USB |

---

*Last updated: 2026-05-18 | Current firmware: `src/sensor_pump_test.cpp` | Current env: `sensor_pump_test`*
