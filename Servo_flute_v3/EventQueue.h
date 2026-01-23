#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <Arduino.h>

// Types d'événements MIDI
enum EventType {
  EVENT_NONE,
  EVENT_NOTE_ON,
  EVENT_NOTE_OFF
};

// Structure d'un événement MIDI avec timestamp
struct MidiEvent {
  EventType type;
  byte midiNote;
  byte velocity;
  unsigned long timestamp;  // Timestamp relatif en ms depuis premier événement

  MidiEvent() : type(EVENT_NONE), midiNote(0), velocity(0), timestamp(0) {}

  MidiEvent(EventType t, byte note, byte vel, unsigned long ts)
    : type(t), midiNote(note), velocity(vel), timestamp(ts) {}
};

// File FIFO circulaire pour événements MIDI
class EventQueue {
public:
  EventQueue(int capacity);

  // Ajoute un événement avec timestamp relatif au premier événement
  bool enqueue(EventType type, byte note, byte velocity, unsigned long absoluteTime);

  // Récupère le prochain événement sans le retirer
  MidiEvent* peek();

  // Retire le prochain événement de la queue
  void dequeue();

  // Vérifie si la queue est vide
  bool isEmpty() const;

  // Vérifie si la queue est pleine
  bool isFull() const;

  // Retourne le nombre d'événements en attente
  int getCount() const;

  // Vide complètement la queue
  void clear();

  // Obtient le timestamp de référence (premier événement)
  unsigned long getReferenceTime() const;

private:
  MidiEvent* _events;
  int _capacity;
  int _head;      // Index d'écriture
  int _tail;      // Index de lecture
  int _count;     // Nombre d'éléments
  unsigned long _referenceTime;  // Timestamp du premier événement (millis absolu)
  bool _hasReference;
};

#endif
