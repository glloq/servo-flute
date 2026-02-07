#include "AirflowController.h"

// Lookup table pour sin() - 256 entrees pour une periode complete [0, 2pi]
// Valeurs: -127 a +127 (represente -1.0 a +1.0)
// Note ESP32 : PROGMEM est ignore (flash mappe en memoire) mais compile sans erreur
const int8_t SIN_LUT[256] PROGMEM = {
  0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46,
  49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88,
  90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
  117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127,
  127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118,
  117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92,
  90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51,
  49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3,
  0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
  -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88,
  -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116,
  -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
  -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118,
  -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100, -98, -96, -94, -92,
  -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
  -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12, -9, -6, -3
};

// Fonction helper pour lookup rapide sin()
// Sur ESP32, pgm_read_byte() est un simple acces memoire (pas de flash separation)
inline float fastSin(unsigned long timeMs, float frequency) {
  unsigned long period = (unsigned long)(1000.0 / frequency);
  unsigned long phase = timeMs % period;
  uint8_t index = (uint8_t)((phase * 256UL) / period);

  return pgm_read_byte(&SIN_LUT[index]) / 127.0;
}

AirflowController::AirflowController(Adafruit_PWMServoDriver& pwm)
  : _pwm(pwm), _solenoidOpen(false), _solenoidOpenTime(0),
    _ccVolume(CC_VOLUME_DEFAULT), _ccExpression(CC_EXPRESSION_DEFAULT), _ccModulation(CC_MODULATION_DEFAULT),
    _ccBreath(CC_BREATH_DEFAULT),
    _cc2BufferIndex(0), _cc2BufferCount(0), _lastCC2Time(0), _lastVelocity(64),
    _baseAngleWithoutVibrato(SERVO_AIRFLOW_OFF), _vibratoActive(false),
    _currentMinAngle(SERVO_AIRFLOW_MIN), _currentMaxAngle(SERVO_AIRFLOW_MAX) {
  for (uint8_t i = 0; i < CC2_SMOOTHING_BUFFER_SIZE; i++) {
    _cc2SmoothingBuffer[i] = CC_BREATH_DEFAULT;
  }
}

void AirflowController::begin() {
  pinMode(SOLENOID_PIN, OUTPUT);
  closeSolenoid();
  setAirflowToRest();

  if (DEBUG) {
    Serial.println("DEBUG: AirflowController - Initialisation");
    #if SOLENOID_USE_PWM
    Serial.println("DEBUG: AirflowController - Mode PWM active");
    Serial.print("DEBUG:   - PWM activation: ");
    Serial.println(SOLENOID_PWM_ACTIVATION);
    Serial.print("DEBUG:   - PWM maintien: ");
    Serial.println(SOLENOID_PWM_HOLDING);
    #else
    Serial.println("DEBUG: AirflowController - Mode GPIO simple");
    #endif
  }
}

void AirflowController::setAirflowVelocity(byte velocity) {
  uint16_t angle;

  if (velocity == 0) {
    angle = SERVO_AIRFLOW_OFF;
  } else {
    angle = map(velocity, 1, 127, SERVO_AIRFLOW_MIN, SERVO_AIRFLOW_MAX);
  }

  setAirflowServoAngle(angle);

  if (DEBUG) {
    Serial.print("DEBUG: AirflowController - Velocite: ");
    Serial.print(velocity);
    Serial.print(" -> Angle: ");
    Serial.println(angle);
  }
}

void AirflowController::setAirflowForNote(byte midiNote, byte velocity) {
  const NoteDefinition* note = getNoteByMidi(midiNote);

  uint16_t minAngle, maxAngle;
  uint16_t baseAngle;

  if (velocity == 0) {
    setAirflowServoAngle(SERVO_AIRFLOW_OFF);
    return;
  }

  if (note != nullptr) {
    minAngle = SERVO_AIRFLOW_MIN + ((SERVO_AIRFLOW_MAX - SERVO_AIRFLOW_MIN) * note->airflowMinPercent / 100);
    maxAngle = SERVO_AIRFLOW_MIN + ((SERVO_AIRFLOW_MAX - SERVO_AIRFLOW_MIN) * note->airflowMaxPercent / 100);
  } else {
    minAngle = SERVO_AIRFLOW_MIN;
    maxAngle = SERVO_AIRFLOW_MAX;
  }

  _currentMinAngle = minAngle;
  _currentMaxAngle = maxAngle;

  // Stocker velocity pour fallback si CC2 timeout
  _lastVelocity = velocity;

  // 1. CC7 (Volume) reduit la limite haute
  float volumeFactor = _ccVolume / 127.0;
  uint16_t effectiveMaxAngle = minAngle + (maxAngle - minAngle) * volumeFactor;

  // 2. Determiner source airflow : CC2 ou velocity
  byte airflowSource;

  #if CC2_ENABLED
  if (_cc2BufferCount > 0) {
    unsigned long timeSinceCC2 = millis() - _lastCC2Time;
    if (CC2_TIMEOUT_MS > 0 && timeSinceCC2 > CC2_TIMEOUT_MS) {
      airflowSource = velocity;
      if (DEBUG) {
        Serial.print("DEBUG: CC2 timeout (");
        Serial.print(timeSinceCC2);
        Serial.print("ms) - Fallback velocity: ");
        Serial.println(velocity);
      }
    } else {
      uint16_t sum = 0;
      for (uint8_t i = 0; i < _cc2BufferCount; i++) {
        sum += _cc2SmoothingBuffer[i];
      }
      byte smoothedCC2 = sum / _cc2BufferCount;

      if (smoothedCC2 < CC2_SILENCE_THRESHOLD) {
        airflowSource = 0;
      } else {
        float normalizedCC2 = smoothedCC2 / 127.0;
        float curvedCC2 = pow(normalizedCC2, CC2_RESPONSE_CURVE);
        airflowSource = (byte)(curvedCC2 * 127);
      }
    }
  } else {
    airflowSource = velocity;
  }
  #else
  airflowSource = velocity;
  #endif

  if (airflowSource == 0) {
    setAirflowServoAngle(SERVO_AIRFLOW_OFF);
    closeSolenoid();
    return;
  }

  // 3. Airflow source definit l'angle de base
  baseAngle = map(airflowSource, 1, 127, minAngle, effectiveMaxAngle);

  // 4. CC11 (Expression) module dans la plage
  float expressionFactor = _ccExpression / 127.0;
  float finalAngleWithoutVibrato = minAngle + (baseAngle - minAngle) * expressionFactor;

  // 5. Limiter
  if (finalAngleWithoutVibrato < SERVO_AIRFLOW_MIN) finalAngleWithoutVibrato = SERVO_AIRFLOW_MIN;
  if (finalAngleWithoutVibrato > SERVO_AIRFLOW_MAX) finalAngleWithoutVibrato = SERVO_AIRFLOW_MAX;

  _baseAngleWithoutVibrato = (uint16_t)(finalAngleWithoutVibrato + 0.5);
  _vibratoActive = (_ccModulation > 0);

  if (DEBUG) {
    Serial.print("DEBUG: AirflowController - Note: ");
    Serial.print(midiNote);
    Serial.print(" | Vel: ");
    Serial.print(velocity);
    Serial.print(" | Range: ");
    Serial.print(minAngle);
    Serial.print("-");
    Serial.print(maxAngle);
    Serial.print(" | Final: ");
    Serial.print(_baseAngleWithoutVibrato);
    Serial.println("deg");
  }

  if (_vibratoActive) {
    update();
  } else {
    setAirflowServoAngle(_baseAngleWithoutVibrato);
  }
}

void AirflowController::openSolenoid() {
  #if SOLENOID_USE_PWM
    setSolenoidPWM(SOLENOID_PWM_ACTIVATION);
    _solenoidOpenTime = millis();
  #else
    if (SOLENOID_ACTIVE_HIGH) {
      digitalWrite(SOLENOID_PIN, HIGH);
    } else {
      digitalWrite(SOLENOID_PIN, LOW);
    }
  #endif

  _solenoidOpen = true;

  if (DEBUG) {
    Serial.println("DEBUG: AirflowController - Solenoide OUVERT");
  }
}

void AirflowController::closeSolenoid() {
  #if SOLENOID_USE_PWM
    setSolenoidPWM(0);
  #else
    if (SOLENOID_ACTIVE_HIGH) {
      digitalWrite(SOLENOID_PIN, LOW);
    } else {
      digitalWrite(SOLENOID_PIN, HIGH);
    }
  #endif

  _solenoidOpen = false;
  _solenoidOpenTime = 0;

  if (DEBUG) {
    Serial.println("DEBUG: AirflowController - Solenoide FERME");
  }
}

bool AirflowController::isSolenoidOpen() const {
  return _solenoidOpen;
}

void AirflowController::setAirflowToRest() {
  setAirflowServoAngle(SERVO_AIRFLOW_OFF);
}

void AirflowController::update() {
  #if SOLENOID_USE_PWM
  if (_solenoidOpen && _solenoidOpenTime > 0) {
    unsigned long elapsed = millis() - _solenoidOpenTime;
    if (elapsed >= SOLENOID_ACTIVATION_TIME_MS) {
      setSolenoidPWM(SOLENOID_PWM_HOLDING);
      _solenoidOpenTime = 0;
    }
  }
  #endif

  // Appliquer vibrato si actif
  if (_vibratoActive && _ccModulation > 0 && _solenoidOpen) {
    const float VIBRATO_FREQUENCY = VIBRATO_FREQUENCY_HZ;
    float vibratoAmplitude = (_ccModulation / 127.0) * VIBRATO_MAX_AMPLITUDE_DEG;
    float vibratoOffset = fastSin(millis(), VIBRATO_FREQUENCY) * vibratoAmplitude;

    int16_t finalAngle = _baseAngleWithoutVibrato + (int16_t)(vibratoOffset + 0.5);

    if (finalAngle < (int16_t)_currentMinAngle) finalAngle = _currentMinAngle;
    if (finalAngle > (int16_t)_currentMaxAngle) finalAngle = _currentMaxAngle;

    setAirflowServoAngle((uint16_t)finalAngle);
  }
}

void AirflowController::setAirflowServoAngle(uint16_t angle) {
  uint16_t pwmValue = angleToPWM(angle);
  _pwm.setPWM(NUM_SERVO_AIRFLOW, 0, pwmValue);
}

uint16_t AirflowController::angleToPWM(uint16_t angle) {
  if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
  if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

  uint16_t pulse = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE,
                       SERVO_PULSE_MIN, SERVO_PULSE_MAX);

  float pulseDuration = (float)pulse / 1000000.0;
  float pwmValue = pulseDuration * SERVO_FREQUENCY * 4096.0;

  return (uint16_t)pwmValue;
}

void AirflowController::setSolenoidPWM(uint8_t pwmValue) {
  #if SOLENOID_USE_PWM
    // ESP32 : analogWrite() est supporte depuis Arduino core 3.x
    if (SOLENOID_ACTIVE_HIGH) {
      analogWrite(SOLENOID_PIN, pwmValue);
    } else {
      analogWrite(SOLENOID_PIN, 255 - pwmValue);
    }
  #endif
}

void AirflowController::setCCValues(byte ccVolume, byte ccExpression, byte ccModulation) {
  _ccVolume = ccVolume;
  _ccExpression = ccExpression;
  _ccModulation = ccModulation;
}

void AirflowController::updateCC2Breath(byte ccBreath) {
  #if CC2_ENABLED
  _cc2SmoothingBuffer[_cc2BufferIndex] = ccBreath;
  _cc2BufferIndex = (_cc2BufferIndex + 1) % CC2_SMOOTHING_BUFFER_SIZE;
  if (_cc2BufferCount < CC2_SMOOTHING_BUFFER_SIZE) {
    _cc2BufferCount++;
  }
  _lastCC2Time = millis();
  _ccBreath = ccBreath;

  if (DEBUG) {
    Serial.print("DEBUG: CC2 (Breath) recu: ");
    Serial.print(ccBreath);
    Serial.print(" | Buffer: ");
    Serial.print(_cc2BufferCount);
    Serial.print("/");
    Serial.println(CC2_SMOOTHING_BUFFER_SIZE);
  }
  #endif
}
