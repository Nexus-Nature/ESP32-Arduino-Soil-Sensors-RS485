# ESP32-Arduino-Soil-Sensors-RS485

![Calibration Data](Experiment_Results_Images/1.JPG)

## Overview
**"Industrial-Soil-Sensors-RS485"** is an open-source collection of drivers, code examples, and benchmark data for various industrial RS485 Modbus soil sensors (including 4-in-1, 7-in-1, 8-in-1 NPK, and standard moisture sensors). 

Many of these sensors lack clear documentation or open-source libraries. This repository solves that by providing:
1.  **Verified Drivers:** Code for Arduino, ESP32, and Python.
2.  **Accuracy Benchmark:** A real-world calibration test comparing 10 different sensor units against gravimetric ground truth.
3.  **Buying Advice:** Based on our tests, we identify the most accurate sensor for professional use.

---

## Calibration Methodology

To ensure reliability, we used the **Gravimetric Method** to determine the actual moisture content of three different soil samples. We prepared samples with specific water weights to calculate the "True Moisture %" using the standard formula:

$$\text{Moisture Content (\%)} = \frac{\text{Wet Soil Weight} - \text{Dry Soil Weight}}{\text{Dry Soil Weight}} \times 100$$

### Reference Values

| Sample ID | Dry Soil Weight (g) | Water Added (g) | **True Moisture (%)** |
| :--- | :---: | :---: | :---: |
| **Sample 1** | 250g | 115g | **46.0%** |
| **Sample 2** | 250g | 61g | **24.5%** |
| **Sample 3** | 250g | 27g | **10.8%** |

---

## Test Results & Comparison

We tested **10 different sensors** against the reference samples above. The table below shows the raw reading from each sensor compared to the real moisture percentage.

| Sensor ID | Sample 1 Reading<br>(Ref: 46.0%) | Sample 2 Reading<br>(Ref: 24.5%) | Sample 3 Reading<br>(Ref: 10.8%) | Accuracy Status |
| :---: | :---: | :---: | :---: | :--- |
| 1 | 92.4 | 36.7 | 20.6 | High Error |
| 2 | 59.8 | 27.2 | 13.3 | Moderate |
| 3 | 53.6 | 25.3 | 13.0 | Good |
| 4 | 76.0 | 31.6 | 17.1 | High Error |
| 5 | 92.7 | 38.2 | 21.3 | High Error |
| **6** | **54.3** | **26.5** | **10.9** | **✅ BEST ACCURACY** |
| 7 | N/A | N/A | N/A | *(NPK Only)* |
| 8 | 50.5 | 30.8 | 16.9 | Inconsistent |
| 9 | 84.9 | 35.5 | 20.0 | High Error |
| 10 | 40.3 | 25.0 | 15.3 | Under-reading |

### Conclusion
As shown in the data, **Sensor #6** provided readings closest to the physical reality (Gravimetric weight).
![Calibration Data](06_Soil_Sensor/6.JPG)


**Recommendation:** We highly recommend using the sensor model found in folder `06_Soil_Sensor` for projects requiring high precision.

#### Experiment Photos
![Setup](Experiment_Results_Images/2.JPG)

---

## Usage
Navigate to the specific sensor folder and upload the `.ino` file to your Arduino, or run the `.py` script on your computer.

```bash
# Example for Python
cd code_python
python SC.py
