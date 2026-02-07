/***********************************************************************************************
----------------------------         SETTINGS ESP32            --------------------------------
Configuration centralisee pour FLUTE IRLANDAISE (Irish Flute en D)
Version ESP32-WROOM avec BLE-MIDI / WiFi-MIDI / Hotspot
Architecture avec servo debit + solenoide valve + mode binaire (ouvert/ferme)
************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H
#include "stdint.h"

#define DEBUG 1

/*******************************************************************************
-------------------------   CONFIGURATION INSTRUMENT  ------------------------
FLUTE IRLANDAISE EN C (Irish Flute / Tin Whistle)
- 6 trous
- Tonalite: C majeur (Do majeur)
- Gamme: A#5 (La#5) a C7 (Do7)
******************************************************************************/

// Nombre de servos pour les doigts
#define NUMBER_SERVOS_FINGER 6

// Nombre de notes jouables
#define NUMBER_NOTES 14

/*******************************************************************************
-----------------------   CONFIGURATION GPIO ESP32    ------------------------
ESP32-WROOM Pin Assignment
******************************************************************************/

// LED d'etat
#define STATUS_LED_PIN 2          // GPIO2 (LED integree sur la plupart des cartes)

// Bouton d'appairage
#define PAIRING_BUTTON_PIN 0      // GPIO0 (bouton BOOT sur les cartes de dev)

// Interrupteur BT/WiFi (HIGH = WiFi, LOW = Bluetooth)
#define MODE_SWITCH_PIN 4         // GPIO4

// I2C (PCA9685)
#define I2C_SDA_PIN 21            // GPIO21 (defaut ESP32)
#define I2C_SCL_PIN 22            // GPIO22 (defaut ESP32)

// Solenoide
#define SOLENOID_PIN 13           // GPIO13

// Alimentation servos (PCA9685 OE pin)
#define PIN_SERVOS_OFF 5          // GPIO5

/*******************************************************************************
---------------------------   TIMING SETTINGS (ms)    ------------------------
******************************************************************************/
// Delai total entre positionnement servos et activation solenoide
#define SERVO_TO_SOLENOID_DELAY_MS  105

// Si deux notes sont espacees de moins que ce delai, on garde la valve ouverte
#define MIN_NOTE_INTERVAL_FOR_VALVE_CLOSE_MS  50

#define MIN_NOTE_DURATION_MS    10

/*******************************************************************************
---------------------------   EVENT QUEUE SETTINGS    ------------------------
******************************************************************************/
#define EVENT_QUEUE_SIZE 16

/*******************************************************************************
---------------------------     SOLENOID VALVE        ------------------------
******************************************************************************/
#define SOLENOID_ACTIVE_HIGH true

// MODE PWM (option pour reduction chaleur)
#define SOLENOID_USE_PWM true

// Parametres PWM (si SOLENOID_USE_PWM = true)
#define SOLENOID_PWM_ACTIVATION 255
#define SOLENOID_PWM_HOLDING    128
#define SOLENOID_ACTIVATION_TIME_MS 50

/*******************************************************************************
---------------------------   AIR FLOW SERVO          ------------------------
******************************************************************************/
#define NUM_SERVO_AIRFLOW 10      // Canal PCA9685 pour servo debit air
#define SERVO_AIRFLOW_OFF 20      // Angle repos (pas de note)
#define SERVO_AIRFLOW_MIN 60      // Angle minimum absolu
#define SERVO_AIRFLOW_MAX 100     // Angle maximum absolu

/*******************************************************************************
---------------------------   POWER MANAGEMENT        ------------------------
******************************************************************************/
#define TIMEUNPOWER 200

/*******************************************************************************
------------------   CONFIGURATION SERVOS DOIGTS       ----------------------
Structure : {PCA_channel, angle_ferme, sens_ouverture}

PCA_channel     : Canal PCA9685 (0-15)
angle_ferme     : Angle servo en position fermee (0-180 deg)
sens_ouverture  : 1 = horaire, -1 = anti-horaire

ORDRE DES TROUS (Irish Flute standard) :
  0 = Trou 1 (haut, main gauche - index)
  1 = Trou 2 (main gauche - majeur)
  2 = Trou 3 (main gauche - annulaire)
  3 = Trou 4 (main droite - index)
  4 = Trou 5 (main droite - majeur)
  5 = Trou 6 (bas, main droite - annulaire)
******************************************************************************/
#define ANGLE_OPEN 30  // Angle d'ouverture du trou (degres)

struct FingerConfig {
  byte pcaChannel;      // Canal PCA9685
  uint16_t closedAngle; // Angle position fermee
  int8_t direction;     // 1=horaire, -1=anti-horaire
};

const FingerConfig FINGERS[NUMBER_SERVOS_FINGER] = {
  // PCA  Ferme  Sens
  {  0,   90,   -1  },  // Trou 1 (haut)
  {  1,   95,    1  },  // Trou 2
  {  2,   90,    1  },  // Trou 3
  {  3,   100,   1  },  // Trou 4
  {  4,   95,   -1  },  // Trou 5
  {  5,   90,    1  }   // Trou 6 (bas)
};

/*******************************************************************************
-----------------   CONFIGURATION DES NOTES JOUABLES   ----------------------
Structure : {MIDI, {doigtes}, flow_min%, flow_max%}
******************************************************************************/

struct NoteDefinition {
  byte midiNote;                            // Numero MIDI
  bool fingerPattern[NUMBER_SERVOS_FINGER]; // Doigtes (0=ferme, 1=ouvert)
  byte airflowMinPercent;                   // % min servo flow (0-100)
  byte airflowMaxPercent;                   // % max servo flow (0-100)
};

// TABLE DES NOTES - Flute irlandaise en C (a partir de A#5)
const NoteDefinition NOTES[NUMBER_NOTES] = {
  // OCTAVE BASSE - Notes graves (A#5 a B5)
  {  82,  {0,1,1,1,1,1},  10,  60  },  // A#5 (La#5)
  {  83,  {1,1,1,1,1,1},  0,   50  },  // B5  (Si5)

  // OCTAVE 1 - MEDIUM (C6 a B6)
  {  84,  {0,0,0,0,0,0},  20,  75  },  // C6  (Do6)
  {  86,  {0,0,0,0,0,1},  15,  70  },  // D6  (Re6)
  {  88,  {0,0,0,0,1,1},  10,  65  },  // E6  (Mi6)
  {  89,  {0,0,0,1,1,1},  10,  60  },  // F6  (Fa6)
  {  91,  {0,0,1,1,1,1},  5,   55  },  // G6  (Sol6)
  {  93,  {0,1,1,1,1,1},  5,   50  },  // A6  (La6)
  {  95,  {1,1,1,1,1,1},  0,   45  },  // B6  (Si6)

  // OCTAVE 2 - AIGU (C7 a G7)
  {  96,  {0,0,0,0,0,0},  50,  100 },  // C7  (Do7)
  {  98,  {0,0,0,0,0,1},  45,  95  },  // D7  (Re7)
  {  100, {0,0,0,0,1,1},  40,  90  },  // E7  (Mi7)
  {  101, {0,0,0,1,1,1},  35,  85  },  // F7  (Fa7)
  {  103, {0,0,1,1,1,1},  30,  80  }   // G7  (Sol7)
};

// Note MIDI la plus basse (calculee automatiquement)
#define FIRST_MIDI_NOTE (NOTES[0].midiNote)

// Fonction utilitaire pour obtenir une note par MIDI
inline const NoteDefinition* getNoteByMidi(byte midiNote) {
  for (int i = 0; i < NUMBER_NOTES; i++) {
    if (NOTES[i].midiNote == midiNote) {
      return &NOTES[i];
    }
  }
  return nullptr;
}

// Fonction utilitaire pour obtenir l'index d'une note
inline int getNoteIndex(byte midiNote) {
  for (int i = 0; i < NUMBER_NOTES; i++) {
    if (NOTES[i].midiNote == midiNote) {
      return i;
    }
  }
  return -1;
}

/*******************************************************************************
-----------------------    SERVO PWM PARAMETERS       ------------------------
******************************************************************************/
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
const uint16_t SERVO_PULSE_MIN = 550;
const uint16_t SERVO_PULSE_MAX = 2450;
const uint16_t SERVO_FREQUENCY = 50;

/*******************************************************************************
-------------------------     MIDI SETTINGS           ------------------------
******************************************************************************/
// Canal MIDI (0 = omni mode, ecoute tous les canaux | 1-16 = canal specifique)
#define MIDI_CHANNEL 0

/*******************************************************************************
-----------------------  CONTROL CHANGE (CC) SETTINGS  -----------------------
******************************************************************************/
#define CC_RATE_LIMIT_PER_SECOND 10

// Vibrato (CC1 - Modulation)
#define VIBRATO_FREQUENCY_HZ 6.0
#define VIBRATO_MAX_AMPLITUDE_DEG 8.0

// Valeurs par defaut CC au demarrage
#define CC_VOLUME_DEFAULT 127
#define CC_EXPRESSION_DEFAULT 127
#define CC_MODULATION_DEFAULT 0
#define CC_BREATH_DEFAULT 127
#define CC_BRIGHTNESS_DEFAULT 64

/*******************************************************************************
--------------------  BREATH CONTROLLER (CC2) SETTINGS  ----------------------
******************************************************************************/
#define CC2_ENABLED true
#define CC2_RATE_LIMIT_PER_SECOND 50
#define CC2_SILENCE_THRESHOLD 10
#define CC2_SMOOTHING_BUFFER_SIZE 5
#define CC2_RESPONSE_CURVE 1.4
#define CC2_TIMEOUT_MS 1000

/*******************************************************************************
-----------------------  WIRELESS SETTINGS (ESP32)    ------------------------
******************************************************************************/

// Nom du peripherique BLE-MIDI et WiFi
#define DEVICE_NAME "ServoFlute"

// WiFi AP mode (hotspot) settings
#define AP_SSID "ServoFlute-Setup"
#define AP_PASSWORD ""                // Pas de mot de passe par defaut (portail ouvert)
#define AP_CHANNEL 1
#define AP_MAX_CONNECTIONS 2

// WiFi STA connection timeout (ms) avant fallback AP
#define WIFI_CONNECT_TIMEOUT_MS 10000

// rtpMIDI port
#define RTPMIDI_PORT 5004

// Web server port
#define WEB_SERVER_PORT 80

// mDNS hostname (accessible via servo-flute.local)
#define MDNS_HOSTNAME "servo-flute"

/*******************************************************************************
-----------------------  HARDWARE INPUT SETTINGS      ------------------------
******************************************************************************/

// Debounce pour bouton et switch (ms)
#define BUTTON_DEBOUNCE_MS 50

// Duree appui long bouton (ms) pour forcer hotspot
#define BUTTON_LONG_PRESS_MS 3000

// Intervalle lecture switch (ms)
#define SWITCH_READ_INTERVAL_MS 100

/*******************************************************************************
-----------------------  STATUS LED SETTINGS          ------------------------
******************************************************************************/

// Patterns LED (durees en ms)
#define LED_BLINK_FAST_MS 100         // Clignotement rapide (advertising/connexion)
#define LED_BLINK_SLOW_MS 1000        // Clignotement lent (connecte, attente MIDI)
#define LED_DOUBLE_FLASH_MS 150       // Double flash (WiFi STA connecte)
#define LED_DOUBLE_FLASH_PAUSE_MS 800 // Pause entre doubles flashs
#define LED_TRIPLE_FLASH_MS 150       // Triple flash (mode hotspot)
#define LED_TRIPLE_FLASH_PAUSE_MS 800 // Pause entre triples flashs

/*******************************************************************************
-----------------------  WATCHDOG SETTINGS (ESP32)    ------------------------
******************************************************************************/
#define WATCHDOG_TIMEOUT_MS 4000      // Timeout watchdog en ms

#endif
