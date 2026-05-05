/*
 * TA1.c - Timer A1 Implementierung
 *
 * Timer A1 wird für zeitkritische Aufgaben verwendet.
 * Typische Anwendungen:
 * - Button-Entprellung (Debouncing)
 * - Periodische Abtastung von Eingängen
 * - Zeitgesteuerte Event-Generierung
 */

#include <msp430.h>
#include "base.h"
#include "TA1.h"
#include "event.h"

// Taktquelle: ACLK = XT1 / 8 = 613,75 kHz
// Zeitspanne = 3,3 ms
// Teilungsfaktor = 613750 * 0,0033 = 2025,375
// Skalierungsfaktor < 1 => {/1} {/1}
// TA1CCR0 = 2025 - 1

#define CNTMAX 6
#define BTNCNT 2

typedef enum { S0, S1 } TState;

LOCAL struct {
   Int    cnt;
   TState state;
} btn[BTNCNT];

LOCAL const struct {
   const UChar * const port;
   const UChar  mask;
   const TEvent msg;
} cfg[BTNCNT] = {
   { (UChar *)(&P1IN), BIT0, EVENT_BTN1 },
   { (UChar *)(&P1IN), BIT1, EVENT_BTN2 }
};

LOCAL UInt i;

/*
 * TA1_init - Initialisiert und startet Timer A1
 *
 * Timer-Konfiguration:
 * - Taktquelle: ACLK (613.75 kHz aus XT1/8)
 * - Modus: Up Mode (zählt von 0 bis TA1CCR0)
 * - Compare Register: 0xFFFF (maximaler Wert)
 * - Interrupt: Aktiviert (Timer Overflow)
 *
 * Die Timer-Periode berechnet sich: T = (CCR0 + 1) / f_ACLK
 * Bei CCR0 = 0xFFFF und f_ACLK = 613.75 kHz: T ≈ 106.7 ms
 */

#pragma FUNC_ALWAYS_INLINE(TA1_init)
#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {


   btn[0].cnt = -(CNTMAX-1); btn[0].state = S0;
   btn[1].cnt = -(CNTMAX-1); btn[1].state = S0;
   i = 0;

   TA1CTL   = 0;
   TA1CCTL0 = 0;
   TA1CCR0  = 2025-1;
   TA1EX0   = TAIDEX_0;       // /1
   TA1CTL   = TASSEL__ACLK
            | MC__UP
            | ID__1            // /1
            | TACLR
            | TAIE;
}


/*
 * TIMER1_A1_ISR - Interrupt Service Routine für Timer A1
 *
 * Diese ISR wird bei Timer-Overflow aufgerufen (wenn TAR = TA1CCR0).
 *
 * AUFGABE: Der Inhalt der ISR ist zu implementieren
 *
 * Mögliche Aufgaben:
 * - Button-Zustände einlesen und entprellen
 * - Bei stabiler Flanke: Event setzen (EVENT_BTN1, EVENT_BTN2)
 * - Zeitgesteuerte Abtastung von Eingängen
 * - Periodische Tasks ausführen
 */
#pragma vector = TIMER1_A1_VECTOR
__interrupt Void TIMER1_A1_ISR(Void) {

   /*
    * TODO: ISR-Implementierung
    * Beispiel für Button-Entprellung:
    * - Timer Interrupt Flag löschen
    * - Button-Pins einlesen (P1IN)
    * - Mit vorherigem Zustand vergleichen
    * - Bei stabiler Änderung: Event_set(EVENT_BTN1/2) aufrufen
    */

   TA1IV;

   if (TSTBIT(*cfg[i].port, cfg[i].mask)) {
      btn[i].cnt   = 0;
      btn[i].state = S0;
   }
   else {
      if (++btn[i].cnt GT CNTMAX-1) {
         btn[i].cnt = CNTMAX-1;
         if (btn[i].state EQ S0) {
            btn[i].state = S1;
            Event_set(cfg[i].msg);
            __low_power_mode_off_on_exit();
         }
      }
   }

   if (++i GE BTNCNT) { i = 0; }

}