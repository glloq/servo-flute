/***********************************************************************************************
 * OUTPUT GENERATOR
 *
 * Génère le code C++ formaté pour copier-coller dans settings.h.
 ***********************************************************************************************/
#ifndef OUTPUT_GENERATOR_H
#define OUTPUT_GENERATOR_H

#include <Arduino.h>
#include "settings_template.h"

class OutputGenerator {
public:
  OutputGenerator();

  // Génère le code C++ complet pour settings.h
  void generateCppCode(const FingerConfig fingers[],
                       const NoteDefinition notes[]);

  // Génère uniquement la section FINGERS
  void generateFingersSection(const FingerConfig fingers[]);

  // Génère uniquement la section NOTES
  void generateNotesSection(const NoteDefinition notes[]);

  // Affiche la configuration actuelle
  void displayCurrentConfig(const FingerConfig fingers[],
                           const NoteDefinition notes[]);

private:
  void printSectionHeader(const char* title);
  void printFingerLine(int index, const FingerConfig& finger);
  void printNoteLine(int index, const NoteDefinition& note);
};

#endif
