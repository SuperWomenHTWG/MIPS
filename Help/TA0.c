/*
 * TA0.c - Timer A0 Implementierung für LED-Blinkmuster
 *
 * Timer A0 steuert verschiedene LED-Blinkmuster.
 * Der Timer läuft kontinuierlich und erzeugt periodische Interrupts.
 */

#include <msp430.h>
#include "base.h"
#include "TA0.h"

/*
 * AUFGABE: Datenstruktur für Blinkmuster
 *
 * Hier soll eine geeignete Datenstruktur überlegt werden,
 * die eine laufzeiteffiziente Ausführung der ISR ermöglicht.
 *
 * Mögliche Ansätze:
 * - Array mit Zeitintervallen pro Muster
 * - Bit-Muster in einer Tabelle
 * - State-Machine mit Zeitwerten
 */

#define HIGH 0x8000
#define LOW 0x0000
#define ACKFRQ 613.75 // kHz
#define TICK(t) ((UInt)(((ACKFRQ * t) / 4.0) / 7.0) - 1)
#define TABSIZE 5


/*
 * set_blink_muster - Wählt das aktive LED-Blinkmuster aus
 *
 * AUFGABE: Diese Funktion muss erweitert werden
 *
 * Die Funktion muss so erweitert werden, dass ein Blinkmuster
 * selektiert wird. Diese Lösung hängt stark von der gewählten
 * Datenstruktur ab.
 *
 * Parameter: arg - Musternummer (MUSTER1 bis MUSTER6)
 */

LOCAL const Int muster1[] = {
      HIGH | TICK(2000),
      LOW  | TICK(500),
      0
   };

    LOCAL const Int muster2[] = {
      HIGH | TICK(750),
      LOW  | TICK(750),
      0
   };

   LOCAL const Int muster3[] = {
      HIGH | TICK(250),
      LOW  | TICK(250),
      0
   };

   LOCAL const Int muster4[] = {
      LOW | TICK(500),
      HIGH  | TICK(500),
      LOW | TICK(1500),
      0
   };

   LOCAL const Int muster5[] = {
      LOW | TICK(500),
      HIGH  | TICK(500),
      LOW | TICK(500),
      HIGH  | TICK(500),
      LOW | TICK(1500),
      0
   };
   LOCAL const Int muster6[] = {
      LOW | TICK(500),
      HIGH  | TICK(500),
      LOW | TICK(500),
      HIGH  | TICK(500),
      LOW | TICK(500),
      HIGH  | TICK(500),
      LOW | TICK(1500),
      0
   };


   // ---------- Auswahltabelle ----------
   LOCAL const Int * const muster_tab[] = {
   muster1, muster2, muster3, muster4, muster5, muster6
};

// ---------- Aktiver Lesezeiger ----------
  LOCAL const Int *ptr       = muster1;  // Default beim Start
   LOCAL const Int *ptr_start = muster1;  // Anfang des aktuellen Musters


GLOBAL Void set_blink_muster(UInt arg) {
   /*
    * TODO: Implementierung des Muster-Wechsels
    * - Muster-Index speichern
    * - Timer-Parameter anpassen
    * - Muster-Daten laden
    */

   ptr_start = muster_tab[arg];
   ptr       = muster_tab[arg];
}
   




/*
 * TA0_init - Initialisiert und startet Timer A0
 *
 * Timer-Konfiguration:
 * - Taktquelle: ACLK (613.75 kHz aus XT1/8)
 * - Modus: Up Mode (zählt von 0 bis TA0CCR0)
 * - Compare Register: 0xFFFF (maximaler Wert)
 * - Interrupt: Aktiviert (Timer Overflow)
 *
 * Die Timer-Periode berechnet sich: T = (CCR0 + 1) / f_ACLK
 * Bei CCR0 = 0xFFFF und f_ACLK = 613.75 kHz: T ≈ 106.7 ms
 */

 
// #pragma FUNC_ALWAYS_INLINE(TA0_init)
// GLOBAL Void TA0_init(Void) {
//    // Timer zunächst stoppen und alle Flags löschen
//    TA0CTL   = 0;

//    // Compare/Capture Control Register 0 konfigurieren
//    TA0CCTL0 = 0;  // Kein Capture-Modus, Compare-Modus
//                   // Interrupt-Flag löschen und deaktivieren

//    // Compare Register setzen (bestimmt Zählperiode)
//    TA0CCR0  = 0xFFFF;       // Maximaler Wert (65535)

//    // Expansion Register (zusätzlicher Teiler)
//    TA0EX0   = TAIDEX_0;     // Teiler /1 (kein zusätzlicher Teiler)

//    // Timer Control Register - Timer starten
//    TA0CTL   = TASSEL__ACLK  // Taktquelle: ACLK (613.75 kHz)
//             | MC__UP        // Up Mode: Zählt von 0 bis CCR0
//             | ID__1         // Input Divider: /1
//             | TACLR         // Timer löschen und starten
//             | TAIE          // Timer Interrupt aktivieren
//             | TAIFG;        // Interrupt Flag setzen (sofortiger erster Interrupt)
// }

#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
   TA0CTL   = 0;            // stop mode, flags löschen
   TA0CCTL0 = 0;            // compare mode, interrupt disabled
   TA0CCR0  = 0;            // wird von ISR beim ersten Aufruf gesetzt
   TA0EX0   = TAIDEX_6;     // Expansion-Teiler /7

   TA0CTL   = TASSEL__ACLK  // 613.75 kHz
            | MC__UP        // Up Mode
            | ID__4         // Input-Teiler /4
            | TACLR         // clear & start
            | TAIE          // overflow interrupt enable
            | TAIFG;        // flag setzen → sofortiger erster Interrupt
}

/*
 * TIMER0_A1_ISR - Interrupt Service Routine für Timer A0
 *
 * Diese ISR wird bei Timer-Overflow aufgerufen (wenn TAR = TA0CCR0).
 *
 * AUFGABE: Der Inhalt der ISR ist zu implementieren
 *
 * Mögliche Aufgaben:
 * - LED-Zustand ändern (toggeln/setzen/löschen)
 * - Blinkmuster-State aktualisieren
 * - Zeitverzögerungen für verschiedene Muster verwalten
 * - Event setzen (wenn Event-System verwendet wird)
 */
#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void) {

   /*
    * TODO: ISR-Implementierung
    * Beispiel:
    * - Timer Interrupt Flag löschen
    * - LED-Pin toggeln
    * - Muster-Logik ausführen
    */

   // Wrap-around: Terminator erreicht → zurück zum Anfang des aktuellen Musters
   if (*ptr EQ 0) {
      ptr = ptr_start;
   }

   // aktuellen Muster-Eintrag holen und ptr auf nächsten Eintrag vorrücken
   UInt cnt = *ptr++;

   // MSB (Bit 15) bestimmt den LED-Pegel
   if (TSTBIT(cnt, HIGH)) {
      SETBIT(P2OUT, BIT7);    // LED1 an
      CLRBIT(cnt, HIGH);      // MSB wegmaskieren → reine Dauer übrig
   } else {
      CLRBIT(P2OUT, BIT7);    // LED1 aus
   }

   // Interrupt-Flag löschen (wird bei TIMER0_A1_VECTOR nicht automatisch gelöscht)
   CLRBIT(TA0CTL, TAIFG);

   // neue Phasendauer als Compare-Wert laden
   TA0CCR0 = cnt;
}
