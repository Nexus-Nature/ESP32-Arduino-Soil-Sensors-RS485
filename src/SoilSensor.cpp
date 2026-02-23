#include "SoilSensor.h"

#define RE_PIN 38
#define DE_PIN 39

static void setRS485Transmit() {
  digitalWrite(RE_PIN, HIGH);
  digitalWrite(DE_PIN, HIGH);
  delayMicroseconds(100);
}

static void setRS485Receive() {
  delayMicroseconds(100);
  digitalWrite(RE_PIN, LOW);
  digitalWrite(DE_PIN, LOW);
}

SoilSensor::SoilSensor(uint8_t modbusAddress)
  : address(modbusAddress),
    profile(ProfileId::Unknown),
    active(nullptr),
    baud(0),
    filterIndex(0),
    isWarmedUp(false) {
  memset(medianFilterTemp, 0, sizeof(medianFilterTemp));
  memset(medianFilterMois, 0, sizeof(medianFilterMois));
}

bool SoilSensor::beginAuto(bool debug) {
  pinMode(RE_PIN, OUTPUT);
  pinMode(DE_PIN, OUTPUT);
  digitalWrite(RE_PIN, LOW);
  digitalWrite(DE_PIN, LOW);

  if (debug) {
    Serial.println("beginAuto(): init RS485 pins done.");
    Serial.flush();
  }

  static const ProbeProfile profiles[] = {
    // 7 regs
    { ProfileId::FULL_7REGS_M0_T1, "7regs M0T1 @9600", 9600, SERIAL_8N1, 0, 7, 0, 1, 2, 3, 4, 5, 6, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },
    { ProfileId::FULL_7REGS_M0_T1, "7regs M0T1 @4800", 4800, SERIAL_8N1, 0, 7, 0, 1, 2, 3, 4, 5, 6, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },
    { ProfileId::FULL_7REGS_T0_M1, "7regs T0M1 @9600", 9600, SERIAL_8N1, 0, 7, 1, 0, 2, 3, 4, 5, 6, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },
    { ProfileId::FULL_7REGS_T0_M1, "7regs T0M1 @4800", 4800, SERIAL_8N1, 0, 7, 1, 0, 2, 3, 4, 5, 6, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },

    // 4 regs
    { ProfileId::MTEP_4REGS_M0_T1, "4regs M0T1 @9600", 9600, SERIAL_8N1, 0, 4, 0, 1, 2, 3, -1,-1,-1, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },
    { ProfileId::MTEP_4REGS_M0_T1, "4regs M0T1 @4800", 4800, SERIAL_8N1, 0, 4, 0, 1, 2, 3, -1,-1,-1, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },
    { ProfileId::MTEP_4REGS_T0_M1, "4regs T0M1 @9600", 9600, SERIAL_8N1, 0, 4, 1, 0, 2, 3, -1,-1,-1, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },
    { ProfileId::MTEP_4REGS_T0_M1, "4regs T0M1 @4800", 4800, SERIAL_8N1, 0, 4, 1, 0, 2, 3, -1,-1,-1, 0.1f, 0.1f, 0.1f, 0.0f, 0,100, -40,85, 3,10, 50000, true, true },

    // 2 regs
    { ProfileId::MT_2REGS_M0_T1, "2regs M0T1 @9600", 9600, SERIAL_8N1, 0, 2, 0, 1, -1,-1,-1,-1,-1, 0.1f, 0.1f, 0.0f, 0.0f, 0,100, -40,85, 0,14, 50000, false, false },
    { ProfileId::MT_2REGS_M0_T1, "2regs M0T1 @4800", 4800, SERIAL_8N1, 0, 2, 0, 1, -1,-1,-1,-1,-1, 0.1f, 0.1f, 0.0f, 0.0f, 0,100, -40,85, 0,14, 50000, false, false },
    { ProfileId::MT_2REGS_T0_M1, "2regs T0M1 @9600", 9600, SERIAL_8N1, 0, 2, 1, 0, -1,-1,-1,-1,-1, 0.1f, 0.1f, 0.0f, 0.0f, 0,100, -40,85, 0,14, 50000, false, false },
    { ProfileId::MT_2REGS_T0_M1, "2regs T0M1 @4800", 4800, SERIAL_8N1, 0, 2, 1, 0, -1,-1,-1,-1,-1, 0.1f, 0.1f, 0.0f, 0.0f, 0,100, -40,85, 0,14, 50000, false, false },

    // NPK only
    { ProfileId::NPK_3REGS_0_2, "3regs NPK @9600", 9600, SERIAL_8N1, 0, 3, -1,-1,-1,-1, 0, 1, 2, 0,0,0,0, 0,100, -40,85, 0,14, 50000, false, false },
    { ProfileId::NPK_3REGS_0_2, "3regs NPK @4800", 4800, SERIAL_8N1, 0, 3, -1,-1,-1,-1, 0, 1, 2, 0,0,0,0, 0,100, -40,85, 0,14, 50000, false, false },
  };

  for (size_t i = 0; i < sizeof(profiles) / sizeof(profiles[0]); i++) {
    if (debug) {
      Serial.print("Trying: ");
      Serial.println(profiles[i].name);
      Serial.flush();
    }

    if (tryProfile(profiles[i], debug)) {
      profile = profiles[i].id;
      active = &profiles[i];
      baud = profiles[i].baud;

      if (debug) {
        Serial.print("Detected OK: ");
        Serial.println(active->name);
        Serial.flush();
      }
      return true;
    }
  }

  profile = ProfileId::Unknown;
  active = nullptr;
  baud = 0;

  if (debug) {
    Serial.println("beginAuto(): no profile matched.");
    Serial.flush();
  }
  return false;
}

bool SoilSensor::tryProfile(const ProbeProfile& p, bool debug) {
  Serial2.begin(p.baud, p.serialConfig, SMT100_RX_PIN, SMT100_TX_PIN);
  delay(120);

  node.begin(address, Serial2);
  node.preTransmission(setRS485Transmit);
  node.postTransmission(setRS485Receive);

  uint16_t regs[16] = {0};

  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    if (probeOnce(p, regs, debug) && valuesLookValid(p, regs)) {
      return true;
    }
    delay(120);
    yield();
  }
  return false;
}

bool SoilSensor::probeOnce(const ProbeProfile& p, uint16_t* outRegs, bool debug) {
  // تنظيف أي بايتات قديمة بالسيريال
  while (Serial2.available()) Serial2.read();

  uint8_t result = node.readHoldingRegisters(p.startReg, p.count);

  if (result != node.ku8MBSuccess) {
    if (debug) {
      Serial.print("  Modbus fail code: ");
      Serial.println(result);
      Serial.flush();
    }
    return false;
  }

  for (uint8_t i = 0; i < p.count; i++) outRegs[i] = node.getResponseBuffer(i);

  if (debug) {
    Serial.print("  RAW: ");
    for (uint8_t i = 0; i < p.count; i++) {
      Serial.print(outRegs[i]);
      if (i + 1 < p.count) Serial.print(",");
    }
    Serial.println();
    Serial.flush();
  }

  return true;
}

float SoilSensor::decodeTemp(const ProbeProfile& p, const uint16_t* regs) const {
  int16_t raw = (int16_t)regs[p.tempIndex];
  return (raw * p.tempScale) + p.tempOffset;
}

float SoilSensor::decodeMois(const ProbeProfile& p, const uint16_t* regs) const {
  return regs[p.moisIndex] * p.moisScale;
}

float SoilSensor::decodePH(const ProbeProfile& p, const uint16_t* regs) const {
  return regs[p.phIndex] * p.phScale;
}

bool SoilSensor::valuesLookValid(const ProbeProfile& p, const uint16_t* regs) const {
  // NPK only
  if (p.nIndex >= 0 && p.pIndex >= 0 && p.kIndex >= 0 && p.moisIndex < 0) {
    uint16_t n = regs[p.nIndex];
    uint16_t phos = regs[p.pIndex];
    uint16_t k = regs[p.kIndex];
    return !(n == 0 && phos == 0 && k == 0);
  }

  // Moist/Temp
  if (p.moisIndex >= 0 && p.tempIndex >= 0) {
    float t = decodeTemp(p, regs);
    float m = decodeMois(p, regs);

    if (!(t >= p.tempMin && t <= p.tempMax)) return false;
    if (!(m >= p.moisMin && m <= p.moisMax)) return false;

    // EC
    if (p.ecIndex >= 0) {
      uint16_t ec = regs[p.ecIndex];
      if (ec > p.ecMax) return false;
      if (p.requireECNonZero && ec == 0) return false;
    }

    // pH
    if (p.phIndex >= 0) {
      float ph = decodePH(p, regs);
      if (!(ph >= p.phMin && ph <= p.phMax)) return false;
      if (p.requirePHNonZero && ph == 0.0f) return false;
    }

    // reduce false positives
    if (t == 0.0f && m == 0.0f) return false;

    return true;
  }

  return false;
}

void SoilSensor::quickWarmup() {
  if (!active) return;
  if (!(active->moisIndex >= 0 && active->tempIndex >= 0)) return;

  for (int i = 0; i < FILTER_WINDOW; i++) {
    uint16_t regs[16] = {0};
    if (probeOnce(*active, regs, false)) {
      medianFilterTemp[i] = decodeTemp(*active, regs);
      medianFilterMois[i] = decodeMois(*active, regs);
    } else {
      medianFilterTemp[i] = 0;
      medianFilterMois[i] = 0;
    }
    delay(150);
    yield();
  }
  isWarmedUp = true;
}

SoilSensor::Reading SoilSensor::read() {
  Reading r{};
  r.isValid = false;

  r.hasMoistTemp = false;
  r.moisture = 0;
  r.temperature = 0;

  r.hasEC = false;
  r.ec = 0;

  r.hasPH = false;
  r.pH = NAN;

  r.hasNPK = false;
  r.n = r.p = r.k = 0;

  if (!active) return r;

  uint16_t regs[16] = {0};
  if (!probeOnce(*active, regs, false)) return r;

  if (active->moisIndex >= 0 && active->tempIndex >= 0) {
    float t = decodeTemp(*active, regs);
    float m = decodeMois(*active, regs);

    r.temperature = isWarmedUp ? calculateMedian(t, medianFilterTemp) : t;
    r.moisture = isWarmedUp ? calculateMedian(m, medianFilterMois) : m;
    r.hasMoistTemp = true;
  }

  if (active->ecIndex >= 0) {
    r.ec = regs[active->ecIndex];
    r.hasEC = true;
  }

  if (active->phIndex >= 0) {
    r.pH = decodePH(*active, regs);
    r.hasPH = true;
  }

  if (active->nIndex >= 0 && active->pIndex >= 0 && active->kIndex >= 0) {
    r.n = regs[active->nIndex];
    r.p = regs[active->pIndex];
    r.k = regs[active->kIndex];
    r.hasNPK = true;
  }

  r.isValid = valuesLookValid(*active, regs);
  return r;
}

float SoilSensor::calculateMedian(float newValue, float* buffer) {
  buffer[filterIndex] = newValue;

  float sortBuffer[FILTER_WINDOW];
  for (int i = 0; i < FILTER_WINDOW; i++) sortBuffer[i] = buffer[i];

  float median = quickSelectMedian(sortBuffer, FILTER_WINDOW);
  filterIndex = (filterIndex + 1) % FILTER_WINDOW;
  return median;
}

float SoilSensor::quickSelectMedian(float arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        float tmp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = tmp;
      }
    }
  }
  if (n % 2 == 0) return (arr[n/2 - 1] + arr[n/2]) / 2.0f;
  return arr[n/2];
} 