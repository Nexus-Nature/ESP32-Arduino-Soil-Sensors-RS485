#ifndef SOILSENSOR_H
#define SOILSENSOR_H

#include <Arduino.h>
#include <ModbusMaster.h>
#include "config.h"

class SoilSensor {
public:
  struct Reading {
    bool isValid;

    bool hasMoistTemp;
    float moisture;
    float temperature;

    bool hasEC;
    uint16_t ec;

    bool hasPH;
    float pH;

    bool hasNPK;
    uint16_t n;
    uint16_t p;
    uint16_t k;
  };

  enum class ProfileId : uint8_t {
    Unknown = 0,
    MT_2REGS_M0_T1,
    MT_2REGS_T0_M1,
    MTEP_4REGS_M0_T1,
    MTEP_4REGS_T0_M1,
    FULL_7REGS_M0_T1,
    FULL_7REGS_T0_M1,
    NPK_3REGS_0_2
  };

  explicit SoilSensor(uint8_t modbusAddress);

  bool beginAuto(bool debug = true);

  ProfileId detectedProfile() const { return profile; }
  uint32_t detectedBaud() const { return baud; }
  const char* detectedProfileName() const { return active ? active->name : "Unknown"; }

  void quickWarmup();
  Reading read();

private:
  struct ProbeProfile {
    ProfileId id;
    const char* name;

    uint32_t baud;
    uint32_t serialConfig;

    uint16_t startReg;
    uint8_t count;

    int moisIndex;
    int tempIndex;
    int ecIndex;
    int phIndex;
    int nIndex;
    int pIndex;
    int kIndex;

    float moisScale;
    float tempScale;
    float phScale;
    float tempOffset;

    float moisMin, moisMax;
    float tempMin, tempMax;
    float phMin, phMax;
    uint16_t ecMax;

    bool requireECNonZero;
    bool requirePHNonZero;
  };

  bool tryProfile(const ProbeProfile& p, bool debug);
  bool probeOnce(const ProbeProfile& p, uint16_t* outRegs, bool debug);

  bool valuesLookValid(const ProbeProfile& p, const uint16_t* regs) const;

  float decodeTemp(const ProbeProfile& p, const uint16_t* regs) const;
  float decodeMois(const ProbeProfile& p, const uint16_t* regs) const;
  float decodePH(const ProbeProfile& p, const uint16_t* regs) const;

  float calculateMedian(float newValue, float* buffer);
  float quickSelectMedian(float arr[], int n);

private:
  ModbusMaster node;
  uint8_t address;

  ProfileId profile;
  const ProbeProfile* active;
  uint32_t baud;

  float medianFilterTemp[FILTER_WINDOW];
  float medianFilterMois[FILTER_WINDOW];
  int filterIndex;
  bool isWarmedUp;
};

#endif