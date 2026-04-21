/*
 * event.h - Event-System Header
 *
 * Dieses Modul implementiert ein ereignisgesteuertes System mit 16 möglichen Events.
 * Jedes Event wird durch ein Bit in einem 16-Bit-Wert repräsentiert.
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "base.h"

// Event-Typ: 16-Bit-Wert, wobei jedes Bit ein Event repräsentiert
typedef unsigned int TEvent;

// Event-Definitionen - Jedes Event hat eine eindeutige Bitposition
#define NO_EVENTS    (0x0000)  // Keine Events (alle Bits gelöscht)
#define EVENT_1      (0x0001)  // Event 1 - Bit 0
#define EVENT_2      (0x0002)  // Event 2 - Bit 1
#define EVENT_3      (0x0004)  // Event 3 - Bit 2
#define EVENT_4      (0x0008)  // Event 4 - Bit 3
#define EVENT_5      (0x0010)  // Event 5 - Bit 4
#define EVENT_6      (0x0020)  // Event 6 - Bit 5
#define EVENT_7      (0x0040)  // Event 7 - Bit 6
#define EVENT_8      (0x0080)  // Event 8 - Bit 7
#define EVENT_9      (0x0100)  // Event 9 - Bit 8
#define EVENT_10     (0x0200)  // Event 10 - Bit 9
#define EVENT_11     (0x0400)  // Event 11 - Bit 10
#define EVENT_12     (0x0800)  // Event 12 - Bit 11
#define EVENT_13     (0x1000)  // Event 13 - Bit 12
#define EVENT_14     (0x2000)  // Event 14 - Bit 13
#define EVENT_15     (0x4000)  // Event 15 - Bit 14
#define EVENT_16     (0x8000)  // Event 16 - Bit 15
#define ALL_EVENTS   (0xFFFF)  // Alle Events (alle 16 Bits gesetzt)


// Event-Zuordnungen für Hardware-Buttons
#define EVENT_BTN1   (EVENT_1)  // Event für Klick auf Button 1
#define EVENT_BTN2   (EVENT_2)  // Event für Klick auf Button 2
#define EVENT_BTN3   (EVENT_3)  // Event für Klick auf Button 3
#define EVENT_BTN4   (EVENT_4)  // Event für Klick auf Button 4
#define EVENT_BTN5   (EVENT_5)  // Event für Klick auf Button 5
#define EVENT_BTN6   (EVENT_6)  // Event für Klick auf Button 6

// Event-System Funktionsdeklarationen

// Initialisiert das Event-System (löscht alle Events)
EXTERN inline Void Event_init(Void);

// Wartet auf Events (versetzt System in Low-Power-Mode wenn keine Events anliegen)
EXTERN inline Void Event_wait(Void);

// Setzt ein oder mehrere Events (markiert sie als aufgetreten)
EXTERN inline Void Event_set(TEvent);

// Löscht ein oder mehrere Events (markiert sie als behandelt)
EXTERN inline Void Event_clr(TEvent);

// Testet ob ein oder mehrere Events aktiv sind
// Rückgabe: TRUE wenn mindestens ein Event gesetzt ist
EXTERN inline Bool Event_tst(TEvent);

// Prüft ob ein Event-Fehler aufgetreten ist
// Rückgabe: TRUE wenn ein Event-Konflikt erkannt wurde
EXTERN inline Bool Event_err(Void);

#endif /* EVENT_H_ */
