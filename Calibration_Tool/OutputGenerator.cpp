/***********************************************************************************************
 * OUTPUT GENERATOR - IMPLEMENTATION
 ***********************************************************************************************/
#include "OutputGenerator.h"

OutputGenerator::OutputGenerator() {
}

void OutputGenerator::generateCppCode(const FingerConfig fingers[],
                                      const NoteDefinition notes[]) {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("CODE GÉNÉRÉ - À COPIER DANS SETTINGS.H"));
  Serial.println(F("========================================"));
  Serial.println();

  // Générer section FINGERS
  generateFingersSection(fingers);

  Serial.println();

  // Générer section NOTES
  generateNotesSection(notes);

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("FIN DU CODE GÉNÉRÉ"));
  Serial.println(F("========================================"));
}

void OutputGenerator::generateFingersSection(const FingerConfig fingers[]) {
  printSectionHeader("CONFIGURATION SERVOS DOIGTS");

  Serial.println(F("const FingerConfig FINGERS[NUMBER_SERVOS_FINGER] = {"));
  Serial.println(F("  // PCA  Fermé  Sens"));

  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    Serial.print(F("  {  "));
    Serial.print(fingers[i].pcaChannel);
    Serial.print(F(",   "));

    if (fingers[i].closedAngle < 100) Serial.print(F(" "));
    Serial.print(fingers[i].closedAngle);
    Serial.print(F(",   "));

    Serial.print(fingers[i].direction);
    Serial.print(F("  }"));

    if (i < NUMBER_SERVOS_FINGER - 1) {
      Serial.print(F(","));
    }

    Serial.print(F("  // Trou "));
    Serial.print(i + 1);

    if (i == 0) Serial.println(F(" (haut)"));
    else if (i == NUMBER_SERVOS_FINGER - 1) Serial.println(F(" (bas)"));
    else Serial.println();
  }

  Serial.println(F("};"));
}

void OutputGenerator::generateNotesSection(const NoteDefinition notes[]) {
  printSectionHeader("CONFIGURATION DES NOTES JOUABLES");

  Serial.println(F("const NoteDefinition NOTES[NUMBER_NOTES] = {"));
  Serial.println(F("  // MIDI  Doigtés (6 trous)  Min%  Max%"));

  for (int i = 0; i < NUMBER_NOTES; i++) {
    Serial.print(F("  {  "));

    // MIDI
    if (notes[i].midiNote < 100) Serial.print(F(" "));
    Serial.print(notes[i].midiNote);
    Serial.print(F(",  {"));

    // Doigtés
    for (int j = 0; j < NUMBER_SERVOS_FINGER; j++) {
      Serial.print(notes[i].fingerPattern[j]);
      if (j < NUMBER_SERVOS_FINGER - 1) Serial.print(F(","));
    }
    Serial.print(F("},  "));

    // Min%
    if (notes[i].airflowMinPercent < 10) Serial.print(F(" "));
    Serial.print(notes[i].airflowMinPercent);
    Serial.print(F(",  "));

    // Max%
    if (notes[i].airflowMaxPercent < 10) Serial.print(F(" "));
    if (notes[i].airflowMaxPercent < 100) Serial.print(F(" "));
    Serial.print(notes[i].airflowMaxPercent);
    Serial.print(F("  }"));

    if (i < NUMBER_NOTES - 1) {
      Serial.print(F(","));
    }

    Serial.print(F("  // "));
    Serial.print(NOTE_NAMES[i]);

    // Ajouter description
    if (i == 0) Serial.println(F(" (La#5)"));
    else if (i == 1) Serial.println(F(" (Si5)"));
    else if (i == 2) Serial.println(F(" (Do6)"));
    else if (i == 3) Serial.println(F(" (Ré6)"));
    else if (i == 4) Serial.println(F(" (Mi6)"));
    else if (i == 5) Serial.println(F(" (Fa6)"));
    else if (i == 6) Serial.println(F(" (Sol6)"));
    else if (i == 7) Serial.println(F(" (La6)"));
    else if (i == 8) Serial.println(F(" (Si6)"));
    else if (i == 9) Serial.println(F(" (Do7)"));
    else if (i == 10) Serial.println(F(" (Ré7)"));
    else if (i == 11) Serial.println(F(" (Mi7)"));
    else if (i == 12) Serial.println(F(" (Fa7)"));
    else if (i == 13) Serial.println(F(" (Sol7)"));
    else Serial.println();
  }

  Serial.println(F("};"));
}

void OutputGenerator::displayCurrentConfig(const FingerConfig fingers[],
                                           const NoteDefinition notes[]) {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("  CONFIGURATION ACTUELLE"));
  Serial.println(F("========================================"));
  Serial.println();

  Serial.println(F("SERVOS DOIGTS:"));
  Serial.println(F("  Trou | PCA | Fermé | Sens | Ouvert"));
  Serial.println(F("  -----|-----|-------|------|-------"));

  for (int i = 0; i < NUMBER_SERVOS_FINGER; i++) {
    Serial.print(F("    "));
    Serial.print(i + 1);
    Serial.print(F("  |  "));
    Serial.print(fingers[i].pcaChannel);
    Serial.print(F("  |  "));

    if (fingers[i].closedAngle < 100) Serial.print(F(" "));
    Serial.print(fingers[i].closedAngle);
    Serial.print(F("°  |  "));

    if (fingers[i].direction == 1) Serial.print(F(" "));
    Serial.print(fingers[i].direction);
    Serial.print(F("   | "));

    uint16_t openAngle = fingers[i].closedAngle + (ANGLE_OPEN * fingers[i].direction);
    if (openAngle < 100) Serial.print(F(" "));
    Serial.print(openAngle);
    Serial.println(F("°"));
  }

  Serial.println();
  Serial.println(F("NOTES JOUABLES:"));
  Serial.println(F("  Note | MIDI | Doigtés    | Min% | Max%"));
  Serial.println(F("  -----|------|------------|------|-----"));

  for (int i = 0; i < NUMBER_NOTES; i++) {
    Serial.print(F("  "));
    Serial.print(NOTE_NAMES[i]);

    if (strlen(NOTE_NAMES[i]) == 2) Serial.print(F(" "));

    Serial.print(F(" |  "));

    if (notes[i].midiNote < 100) Serial.print(F(" "));
    Serial.print(notes[i].midiNote);
    Serial.print(F("  | "));

    for (int j = 0; j < NUMBER_SERVOS_FINGER; j++) {
      Serial.print(notes[i].fingerPattern[j]);
    }
    Serial.print(F("     |  "));

    if (notes[i].airflowMinPercent < 10) Serial.print(F(" "));
    Serial.print(notes[i].airflowMinPercent);
    Serial.print(F("%  |  "));

    if (notes[i].airflowMaxPercent < 10) Serial.print(F(" "));
    if (notes[i].airflowMaxPercent < 100) Serial.print(F(" "));
    Serial.print(notes[i].airflowMaxPercent);
    Serial.println(F("%"));
  }

  Serial.println();
}

void OutputGenerator::printSectionHeader(const char* title) {
  Serial.println(F("/*******************************************************************************"));
  Serial.print(F("------------------   "));
  Serial.print(title);
  Serial.println(F("       ----------------------"));
  Serial.println(F("******************************************************************************/"));
}
