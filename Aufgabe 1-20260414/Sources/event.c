/*
 * event.c - Event-System Implementierung
 *
 * Implementiert ein ereignisgesteuertes System mit drei Event-Registern:
 * - nxt: Neue Events, die noch nicht verarbeitet wurden
 * - act: Aktive Events, die gerade verarbeitet werden
 * - err: Events, die doppelt gesetzt wurden (Fehlerfall)
 */

#include <msp430.h>
#include "event.h"

// Lokale Event-Verwaltungsstruktur
// volatile: Compiler-Optimierung verhindern (wird von ISRs verändert)
LOCAL volatile struct {
   TEvent nxt;  // Nächste Events (von ISRs gesetzt)
   TEvent act;  // Aktive Events (zur Verarbeitung in Main-Loop)
   TEvent err;  // Fehler-Events (doppelt gesetzt = nicht rechtzeitig verarbeitet)
} evn;

/*
 * Event_init - Initialisiert das Event-System
 *
 * Löscht alle Event-Register (nxt, act, err).
 * Sollte beim Systemstart aufgerufen werden.
 */
#pragma FUNC_ALWAYS_INLINE(Event_init)
GLOBAL inline Void Event_init(Void) {
   CLRBIT(evn.nxt, ALL_EVENTS);  // Lösche alle zukünftigen Events
   CLRBIT(evn.act, ALL_EVENTS);  // Lösche alle aktiven Events
   CLRBIT(evn.err, ALL_EVENTS);  // Lösche alle Event-Fehler
}

/*
 * Event_wait - Wartet auf Events und verwaltet Low-Power-Mode
 *
 * Diese Funktion:
 * 1. Überträgt neue Events (nxt) zu aktiven Events (act)
 * 2. Erkennt Event-Fehler (wenn ein Event noch aktiv ist und erneut gesetzt wird)
 * 3. Versetzt System in Low-Power-Mode 3, wenn keine Events vorliegen
 * 4. Interrupt-sicher durch disable/enable-Paar
 */
#pragma FUNC_ALWAYS_INLINE(Event_wait)
GLOBAL inline Void Event_wait(Void) {
   _disable_interrupt();  // Kritischer Abschnitt beginnt

   // Fehler-Erkennung: Events die noch aktiv sind und neu gesetzt wurden
   SETBIT(evn.err, evn.act BAND evn.nxt);

   // Neue Events zu aktiven Events übertragen
   SETBIT(evn.act, evn.nxt);

   // Neue Events löschen (wurden zu aktiven Events)
   CLRBIT(evn.nxt, ALL_EVENTS);

   // Wenn keine Events aktiv: Low-Power-Mode aktivieren
   if (evn.act EQ NO_EVENTS) {
      CLRBIT(P3OUT, BIT7);      // TEST4-Pin LOW (Debug-Signal)
      _low_power_mode_3();      // CPU schlafen legen (Interrupt weckt auf)
      SETBIT(P3OUT, BIT7);      // TEST4-Pin HIGH (CPU ist wach)
   }

   _enable_interrupt();  // Kritischer Abschnitt endet
}

/*
 * Event_set - Setzt ein oder mehrere Events
 *
 * Wird typischerweise von Interrupt Service Routines (ISRs) aufgerufen,
 * um Events zu signalisieren.
 *
 * Parameter: arg - Event(s) die gesetzt werden sollen (Bitmask)
 */
#pragma FUNC_ALWAYS_INLINE(Event_set)
GLOBAL inline Void Event_set(TEvent arg) {
   SETBIT(evn.nxt, arg);  // Event in "nächste Events" setzen
}

/*
 * Event_clr - Löscht ein oder mehrere aktive Events
 *
 * Wird nach der Verarbeitung eines Events aufgerufen,
 * um es als "erledigt" zu markieren.
 *
 * Parameter: arg - Event(s) die gelöscht werden sollen (Bitmask)
 */
#pragma FUNC_ALWAYS_INLINE(Event_clr)
GLOBAL inline Void Event_clr(TEvent arg) {
   CLRBIT(evn.act, arg);  // Event aus "aktiven Events" löschen
}

/*
 * Event_tst - Testet ob ein oder mehrere Events aktiv sind
 *
 * Parameter: arg - Event(s) die getestet werden sollen (Bitmask)
 * Rückgabe: TRUE wenn mindestens eines der Events aktiv ist
 */
#pragma FUNC_ALWAYS_INLINE(Event_tst)
GLOBAL inline Bool Event_tst(TEvent arg) {
   return TSTBIT(evn.act, arg);  // Gibt TRUE zurück wenn Event(s) gesetzt
}

/*
 * Event_err - Prüft ob ein Event-Fehler vorliegt
 *
 * Ein Fehler tritt auf, wenn ein Event erneut gesetzt wird,
 * bevor es verarbeitet wurde (Event-Overflow).
 *
 * Rückgabe: TRUE wenn ein Fehler vorliegt
 */
#pragma FUNC_ALWAYS_INLINE(Event_err)
GLOBAL inline Bool Event_err(Void) {
   return (evn.err NE NO_EVENTS);  // TRUE wenn Fehler-Events vorhanden
}
