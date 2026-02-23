# Developer Guide

Welcome to the technical documentation for the ESP32 RS485 Soil Sensor project. This guide explains the hardware configuration, the architecture of the C++ auto-detection library, and the usage of the Python debugging scripts.

---

## 1. Hardware & Wiring Configuration (`config.h` & `SoilSensor.cpp`)

The system uses a MAX485 (or equivalent) TTL-to-RS485 transceiver to communicate with the sensors. The pinout is explicitly defined to support power cycling and signal stability.

* **UART Pins:** RX is mapped to pin `40`, and TX is mapped to pin `41`.
* **RS485 Control Pins:** The Receive Enable (`RE_PIN`) is on pin `38`, and Data Enable (`DE_PIN`) is on pin `39`. The library actively toggles these pins during transmit/receive cycles via `setRS485Transmit()` and `setRS485Receive()`.
* **Power Management:** To handle sensors that occasionally hang, the ESP32 can hard-reset the sensor or transceiver using the `IC_ENABLE` (pin `21`) and `VCC_ENABLE` (pin `14`) pins.

---

## 2. Core C++ Library Architecture (`SoilSensor.h` & `SoilSensor.cpp`)

The core innovation of this library is its ability to bypass hardcoded registers and dynamically adapt to the connected sensor.

### The Auto-Detect Engine (`beginAuto`)
Chinese sensors differ wildly in their Modbus register mappings and baud rates. The `SoilSensor.h` defines a `ProbeProfile` struct that maps exactly where each parameter (Moisture, Temp, EC, pH, NPK) is located, along with its scaling factor. 

When `sensor.beginAuto(true)` is called, the library executes the following logic:
1.  Loops through a predefined array of known profiles (e.g., `FULL_7REGS_M0_T1`, `MT_2REGS_T0_M1`).
2.  Tests standard baud rates (9600 and 4800) for each profile.
3.  Reads the registers and validates the response against strict mathematical bounds (e.g., Moisture between 0-100%, pH between 3-10) using `valuesLookValid()`.
4.  Locks in the correct profile and baud rate once a valid match is found.

### DSP & Signal Smoothing (`calculateMedian`)
To mitigate electrical noise common in RS485 environments, the library implements a QuickSelect median filter. It stores a rolling buffer of the last 10 readings (`FILTER_WINDOW = 10`) for Temperature and Moisture, ensuring that sudden spikes are ignored.

---

## 3. Python Debugging Utilities

Sometimes, working directly with the ESP32 makes debugging Modbus responses difficult. We provided two Python scripts using the `minimalmodbus` library to interface with the sensors directly from a PC via a USB-to-RS485 adapter.

### A. Finding the Slave ID (`minimal_scan.py`)
If a sensor's Modbus Slave ID is unknown, `minimal_scan.py` iterates from ID `1` to `247` at 4800 baud. It attempts to read register 0, and stops as soon as it receives a valid response, revealing the hidden ID.

### B. Profile Detection (`auto_soil_sensor.py`)
Once the ID is known, `auto_soil_sensor.py` acts as a Python version of our C++ auto-detect engine. It iterates through a dictionary of `PROFILES` (testing different baud rates and register alignments) and prints out the decoded values for Moisture, Temperature, EC, pH, and NPK if successful.