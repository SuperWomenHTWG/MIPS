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
#include "..\base.h"
#include "TA1.h"
#include "event.h"


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

 #define CNTMAX 5

typedef enum { S0, S1 } TState;

// --- Variable pro Button (RAM, ändert sich) ---
LOCAL struct { Int cnt; TState state; } var1, var2, var3, var4, var5, var6;

// --- Konstante pro Button (FRAM, fest) ---
LOCAL const struct {
   const UChar * const port;
   const UChar         mask;
   const TEvent        msg;
} btn1 = { (UChar *)(&P1IN), BIT0, EVENT_BTN1 },
  btn2 = { (UChar *)(&P1IN), BIT1, EVENT_BTN2 },
  btn3 = { (UChar *)(&P3IN), BIT0, EVENT_BTN3 },
  btn4 = { (UChar *)(&P3IN), BIT1, EVENT_BTN4 },
  btn5 = { (UChar *)(&P3IN), BIT2, EVENT_BTN5 },
  btn6 = { (UChar *)(&P3IN), BIT3, EVENT_BTN6 };

// --- Entprell-Logik als Makro (wird zur Compile-Zeit expandiert) ---
#define DEBOUNCE(b, v)                          \
   if (TSTBIT(*(b).port, (b).mask)) {           \
      if (--(v).cnt LT 0) {                     \
         (v).cnt   = 0;                         \
         (v).state = S0;                        \
      }                                         \
   } else if (++(v).cnt GT CNTMAX-1) {          \
      (v).cnt = CNTMAX-1;                       \
      if ((v).state EQ S0) {                    \
         (v).state = S1;                        \
         Event_set((b).msg);                    \
         wake = 1;                              \
      }                                         \
   }
   
#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {

   // Timer zunächst stoppen und alle Flags löschen
   TA1CTL   = 0;

   // Compare/Capture Control Register 0 konfigurieren
   TA1CCTL0 = 0;  // Kein Capture-Modus, Compare-Modus
                  // Interrupt-Flag löschen und deaktivieren

   // Compare Register setzen (bestimmt Zählperiode)
   TA1CCR0  = 0xFFFF;          // Maximaler Wert (65535)

   // Expansion Register (zusätzlicher Teiler)
   TA1EX0   = TAIDEX_0;        // Teiler /1 (kein zusätzlicher Teiler)

   // Timer Control Register - Timer starten
   TA1CTL   = TASSEL__ACLK     // Taktquelle: ACLK (613.75 kHz)
            | MC__UP           // Up Mode: Zählt von 0 bis CCR0
            | ID__1            // Input Divider: /1
            | TACLR            // Timer löschen und starten
            | TAIE;            // Timer Interrupt aktivieren
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

   CLRBIT(TA1CTL, TAIFG);
      UChar wake = 0;

      DEBOUNCE(btn1, var1)
      DEBOUNCE(btn2, var2)
      DEBOUNCE(btn3, var3)
      DEBOUNCE(btn4, var4)
      DEBOUNCE(btn5, var5)
      DEBOUNCE(btn6, var6)

      if (wake) {
         __low_power_mode_off_on_exit();
      }  

}
