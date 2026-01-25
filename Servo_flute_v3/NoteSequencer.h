#ifndef NOTE_SEQUENCER_H
#define NOTE_SEQUENCER_H

#include <Arduino.h>
#include "EventQueue.h"
#include "FingerController.h"
#include "AirflowController.h"
#include "settings.h"

// États de la machine à états pour une note
enum NoteState {
  STATE_IDLE,              // Aucune note en cours
  STATE_POSITIONING,       // Servos en déplacement + attente stabilisation
  STATE_PLAYING,           // Note active, son produit
  STATE_STOPPING           // Arrêt en cours
};

class NoteSequencer {
public:
  NoteSequencer(EventQueue& eventQueue, FingerController& fingerCtrl, AirflowController& airflowCtrl);

  // Démarre le séquenceur
  void begin();

  // Méthode update appelée dans loop() - gère la state machine
  void update();

  // Retourne l'état actuel
  NoteState getState() const;

  // Retourne true si une note est en train d'être jouée
  bool isPlaying() const;

  // Arrête immédiatement toute lecture (pour All Sound Off)
  void stop();

private:
  EventQueue& _eventQueue;
  FingerController& _fingerCtrl;
  AirflowController& _airflowCtrl;

  NoteState _currentState;
  byte _currentNote;
  byte _currentVelocity;
  unsigned long _stateStartTime;      // Timestamp de début de l'état actuel
  unsigned long _eventScheduledTime;  // Timestamp absolu où l'événement doit être joué
  unsigned long _playbackStartTime;   // Timestamp de début de la lecture (millis absolu)

  // Traite le prochain événement dans la queue
  void processNextEvent();

  // Transition vers un nouvel état
  void transitionTo(NoteState newState);

  // Gère l'état IDLE
  void handleIdle();

  // Gère l'état POSITIONING (servos + attente stabilisation)
  void handlePositioning();

  // Gère l'état PLAYING
  void handlePlaying();

  // Gère l'état STOPPING
  void handleStopping();

  // Démarre la séquence de jeu d'une note
  void startNoteSequence(byte note, byte velocity, unsigned long scheduledTime);

  // Arrête la note en cours
  void stopCurrentNote();

  // Vérifie s'il faut fermer la valve entre deux notes (optimisation)
  bool shouldCloseValveBetweenNotes();
};

#endif
