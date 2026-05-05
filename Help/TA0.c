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

// Taktquelle: ACLK = XT1 / 8 = 613,75 kHz
// Laengste Phase: 2,0 s
// Teilungsfaktor = 613750 * 2,0 = 1227500
// 15-Bit nutzbar (MSB = HIGH/LOW): max 32768
// Skalierungsfaktor = 1227500 / 32768 = 37,46 => 40 = {/8} {/5}

#define HIGH    0x8000
#define LOW     0x0000
#define ACKFRQ  613.75  // kHz
#define TICK(t) ((UInt)(((ACKFRQ * t) / 8.0) / 5.0) - 1)

LOCAL const UInt m1[] = { // (1) unterbrochenes Licht
   HIGH | TICK(2000), LOW | TICK(500), 0
};
LOCAL const UInt m2[] = { // (2) langsames Gleichtaktlicht
   HIGH | TICK(750), LOW | TICK(750), 0
};
LOCAL const UInt m3[] = { // (3) schnelles Gleichtaktlicht
   HIGH | TICK(250), LOW | TICK(250), 0
};
LOCAL const UInt m4[] = { // (4) 1x Blinken
   HIGH | TICK(500), LOW | TICK(2000), 0
};
LOCAL const UInt m5[] = { // (5) 2x Blinken
   HIGH | TICK(500), LOW | TICK(500),
   HIGH | TICK(500), LOW | TICK(1500), 0
};
LOCAL const UInt m6[] = { // (6) 3x Blinken
   HIGH | TICK(500), LOW | TICK(500),
   HIGH | TICK(500), LOW | TICK(500),
   HIGH | TICK(500), LOW | TICK(1500), 0
};

LOCAL const UInt * const muster_tab[] = {
   m1, m2, m3, m4, m5, m6
};

LOCAL const UInt * volatile ptr;
LOCAL const UInt * volatile pending;

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
GLOBAL Void set_blink_muster(UInt arg) {
   pending = muster_tab[arg];
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
#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
   ptr     = muster_tab[MUSTER1];
   pending = muster_tab[MUSTER1];

   TA0CTL   = 0;
   TA0CCTL0 = 0;
   TA0CCR0  = TICK(2000);
   TA0EX0   = TAIDEX_4;       // /5
   TA0CTL   = TASSEL__ACLK
            | MC__UP
            | ID__8            // /8!!
            | TACLR
            | TAIE
            | TAIFG;

   ptr     = muster_tab[MUSTER1];  // Nach einem Spannungsverlust - Pointer neu setzen
   pending = muster_tab[MUSTER1];
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
__interrupt Void TIMER0_A1_ISR(Void)
{
   /*
    * TODO: ISR-Implementierung
    * Beispiel:
    * - Timer Interrupt Flag löschen
    * - LED-Pin toggeln
    * - Muster-Logik ausführen
    */
   TA0IV;

   if (*ptr EQ 0) { ptr = pending; }

   UInt cnt = *ptr++;

   CLRBIT(P1OUT, BIT2);

   if (TSTBIT(cnt, HIGH)) { SETBIT(P1OUT, BIT2); }
   CLRBIT(cnt, HIGH);
   TA0CCR0 = cnt;
}