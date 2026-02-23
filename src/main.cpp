#include <Arduino.h>
#include "config.h"
#include "SoilSensor.h"

SoilSensor sensor(1);

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(IC_ENABLE, OUTPUT);
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(IC_ENABLE, HIGH);
  digitalWrite(VCC_ENABLE, HIGH);

  Serial.println("Starting SoilSensor auto-detect...");

  if (!sensor.beginAuto(true)) {
    Serial.println("Auto-detect failed. Check wiring / slave id / baud / parity / pins.");
  } else {
    Serial.print("Detected baud: ");
    Serial.println(sensor.detectedBaud());
    Serial.print("Detected profile: ");
    Serial.println(sensor.detectedProfileName());

    sensor.quickWarmup();
    Serial.println("Warmup done.");
  }
}

void loop() {
  auto r = sensor.read();

  if (!r.isValid) {
    Serial.println("Read failed / invalid.");
    delay(1000);
    return;
  }

  bool printed = false;

  if (r.hasMoistTemp) {
    Serial.print("Temp(C): ");
    Serial.print(r.temperature, 1);
    Serial.print(" | Moisture: ");
    Serial.print(r.moisture, 1);
    printed = true;
  }

  if (r.hasEC) {
    if (printed) Serial.print(" | ");
    Serial.print("EC: ");
    Serial.print(r.ec);
    printed = true;
  }

  if (r.hasPH) {
    if (printed) Serial.print(" | ");
    Serial.print("pH: ");
    Serial.print(r.pH, 1);
    printed = true;
  }

  if (r.hasNPK) {
    if (printed) Serial.print(" | ");
    Serial.print("N: ");
    Serial.print(r.n);
    Serial.print(" P: ");
    Serial.print(r.p);
    Serial.print(" K: ");
    Serial.print(r.k);
    printed = true;
  }

  if (!printed) Serial.print("No fields.");
  Serial.println();

  delay(1000);
}