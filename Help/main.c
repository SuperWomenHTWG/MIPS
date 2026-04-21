/*
 * main.c - Hauptprogramm für MSP430 LED-Blinkmuster-Steuerung
 *
 * Dieses Programm implementiert:
 * - Ereignisgesteuerte Architektur mit Event-System
 * - LED-Blinkmuster steuerbar über Buttons
 * - Timer-basierte Steuerung (TA0 für Blinken, TA1 für Button-Entprellung)
 * - Low-Power-Mode Unterstützung
 */

#include <msp430.h>
#include "base.h"
#include "event.h"
#include "TA0.h"
#include "TA1.h"

/*
 * _system_pre_init - Wird vor allen Initialisierungen aufgerufen
 *
 * Diese Funktion wird vom C-Runtime-System vor main() und vor
 * der Initialisierung von globalen Variablen aufgerufen.
 *
 * Hauptaufgabe: Watchdog-Timer deaktivieren
 * (Verhindert automatischen System-Reset)
 *
 * Rückgabe: 0 = Normale Initialisierung fortsetzen
 */
GLOBAL Int _system_pre_init(Void) {
   // Watchdog Timer stoppen (verhindert ungewollte Resets)
   // WDTPW = Watchdog Password (erforderlich für Schreibzugriff)
   // WDTHOLD = Watchdog Hold (stoppt den Timer)
   WDTCTL = WDTPW + WDTHOLD;
   return 0;  // Normale Initialisierung fortsetzen
}

/*
 * CS_init - Clock System Initialisierung
 *
 * Konfiguriert die drei System-Takte des MSP430:
 *
 * Taktquellen:
 * - DCO (Digitally Controlled Oscillator): 8.0 MHz (intern)
 * - XT1 (Externer Kristall): 4.91 MHz (extern)
 *
 * System-Takte:
 * - ACLK  (Auxiliary Clock)  := XT1 / 8  = 613.75 kHz  (für Timer, langsam & präzise)
 * - SMCLK (Sub-Main Clock)   := DCO / 32 = 250.0 kHz   (für Peripherie)
 * - MCLK  (Main Clock)       := DCO / 1  = 8.0 MHz     (für CPU)
 */
#pragma FUNC_ALWAYS_INLINE(CS_init)
LOCAL inline Void CS_init(Void) {
   // Clock System freischalten (Password erforderlich)
   CSCTL0_H = CSKEY_H;

   // DCO Frequenz einstellen
   CSCTL1 = DCOFSEL_3;    // DCO Frequenz = 8.0 MHz

   // Taktquellen für die System-Takte auswählen
   CSCTL2 = SELA__XT1CLK  // ACLK  <- XT1 (externer Kristall)
          | SELS__DCOCLK  // SMCLK <- DCO (interner Oszillator)
          | SELM__DCOCLK; // MCLK  <- DCO (interner Oszillator)

   // Frequenzteiler für die System-Takte einstellen
   CSCTL3 = DIVA__8       // ACLK  : /8  (4.91 MHz / 8 = 613.75 kHz)
          | DIVS__32      // SMCLK : /32 (8.0 MHz / 32 = 250.0 kHz)
          | DIVM__1;      // MCLK  : /1  (8.0 MHz / 1 = 8.0 MHz)

   // Oszillator-Konfiguration
   CSCTL4 = XT2OFF        // XT2 deaktiviert (nicht verwendet)
          | XTS           // XT1 HF-Modus (High Frequency, > 3 MHz)
          | XT1DRIVE_0;   // XT1 Low Power, kein Bypass

   // Fehler-Flags deaktivieren und löschen
   CSCTL5 = 0;

   // Bedingte Takt-Anforderungen deaktivieren
   CSCTL6 = 0;

   // Clock System wieder sperren
   CSCTL0_H = 0;
}

/*
 * VAL_16BIT - Hilfsmakro für 16-Bit-Werte aus zwei 8-Bit-Werten
 *
 * Der MSP430 verwendet oft gepaarte Ports (z.B. P1 & P2 = PA).
 * Dieses Makro kombiniert zwei 8-Bit-Werte zu einem 16-Bit-Wert.
 *
 * Parameter: arg1 = High-Byte, arg2 = Low-Byte
 * Beispiel: VAL_16BIT(0xAB, 0xCD) = 0xABCD
 */
#define VAL_16BIT(arg1, arg2) ((unsigned)(((arg1) << 8) | (arg2)))

/*
 * GPIO_init - GPIO (General Purpose Input/Output) Initialisierung
 *
 * Konfiguriert alle verwendeten Pins des MSP430:
 *
 * Ausgänge (Outputs):
 * - Port 3, Pin 7 => TEST4 (Debug-Signal, CPU-Aktivität)
 * - Port 3, Pin 6 => TEST3 (Debug-Signal)
 * - Port 3, Pin 5 => TEST2 (Debug-Signal)
 * - Port 3, Pin 4 => TEST1 (Debug-Signal)
 * - Port 2, Pin 7 => LED1 (LED-Anzeige)
 * - Port 1, Pin 2 => LED2 (LED-Anzeige, Fehler-Indikator)
 *
 * Eingänge (Inputs):
 * - Port 1, Pin 0 => BTN1 (Button 1, LED toggeln)
 * - Port 1, Pin 1 => BTN2 (Button 2, Muster wechseln)
 */
#pragma FUNC_ALWAYS_INLINE(GPIO_init)
LOCAL inline Void GPIO_init(Void) {

   // Port A Konfiguration (Port A = Port 2 & Port 1 kombiniert)
   //                   Port2       Port1
   //               Bit 76543210    76543210
   //                   7-------    --2-----  = Ausgänge
   //                   --------    01------  = Eingänge

   // PAOUT: Output Register - Ausgangswerte setzen
   PAOUT  = VAL_16BIT(0b00000000, 0b00000000); // Alle Ausgänge auf LOW

   // PADIR: Direction Register - Datenrichtung festlegen
   PADIR  = VAL_16BIT(0b10000000, 0b00000100); // 1=Output, 0=Input
                                                // P2.7=LED1, P1.2=LED2 als Output
                                                // P1.0=BTN1, P1.1=BTN2 als Input

   // PAREN: Resistor Enable - Pull-Up/Down-Widerstände
   PAREN  = VAL_16BIT(0b00000000, 0b00000000); // Alle deaktiviert (keine Pull-Ups)

   // PAIFG: Interrupt Flag - Interrupt-Flags löschen
   PAIFG  = VAL_16BIT(0b00000000, 0b00000000); // Alle Flags gelöscht

   // PAIES: Interrupt Edge Select - Flankenauswahl für Interrupts
   PAIES  = VAL_16BIT(0b00000000, 0b00000000); // Nicht verwendet (0=H->L, 1=L->H)

   // PAIE: Interrupt Enable - GPIO-Interrupts aktivieren
   PAIE   = VAL_16BIT(0b00000000, 0b00000000); // Alle deaktiviert (Button-Polling via Timer)

   // PASEL0/1: Function Select - Spezialfunktionen auswählen
   PASEL0 = VAL_16BIT(0b00000000, 0b00000000); // 00 = GPIO-Modus
   PASEL1 = VAL_16BIT(0b00000000, 0b00000000); // (andere Werte: UART, SPI, etc.)

   // Port B Konfiguration (Port B = Port 4 & Port 3 kombiniert)
   //                   Port4       Port3
   //               Bit 76543210    76543210
   //                   --------    7654----  = Ausgänge (TEST-Signale)

   // PBOUT: Output Register - Ausgangswerte setzen
   PBOUT  = VAL_16BIT(0b00000000, 0b10000000); // P3.7=TEST4 auf HIGH (CPU aktiv)
                                                // Andere auf LOW

   // PBDIR: Direction Register - Datenrichtung festlegen
   PBDIR  = VAL_16BIT(0b00000000, 0b11110000); // P3.4-7 = Ausgänge (TEST1-4)

   // PBREN: Resistor Enable - Pull-Up/Down-Widerstände
   PBREN  = VAL_16BIT(0b00000000, 0b00000000); // Alle deaktiviert

   // PBIFG: Interrupt Flag - Interrupt-Flags löschen
   PBIFG  = VAL_16BIT(0b00000000, 0b00000000); // Alle Flags gelöscht

   // PBIES: Interrupt Edge Select - Flankenauswahl
   PBIES  = VAL_16BIT(0b00000000, 0b00000000); // Nicht verwendet

   // PBIE: Interrupt Enable - GPIO-Interrupts
   PBIE   = VAL_16BIT(0b00000000, 0b00000000); // Alle deaktiviert

   // PBSEL0/1: Function Select - GPIO-Modus
   PBSEL0 = VAL_16BIT(0b00000000, 0b00000000); // 00 = GPIO-Modus
   PBSEL1 = VAL_16BIT(0b00000000, 0b00000000);
}

/*
 * main - Hauptfunktion des Programms
 *
 * Ablauf:
 * 1. System-Initialisierung (GPIO, Clock, Events, Timer)
 * 2. Endlos-Schleife mit Event-gesteuerter Verarbeitung
 *
 * Event-Loop:
 * - EVENT_BTN2: Wechselt zum nächsten Blinkmuster (MUSTER1-6)
 * - EVENT_BTN1: Toggelt LED1 (P2.7)
 * - Event_err(): Zeigt Event-Overflow-Fehler an (LED2 leuchtet)
 */
GLOBAL Void main(Void) {
   // Zähler für aktuelles Blinkmuster (MUSTER1 bis MUSTER6)
   Int cnt = MUSTER1;

   // System-Initialisierung
   GPIO_init();   // GPIO-Pins konfigurieren
   CS_init();     // Clock System konfigurieren
   Event_init();  // Event-System initialisieren
   TA0_init();    // Timer A0 starten (Blinkmuster)
   TA1_init();    // Timer A1 starten (Button-Entprellung)

   // Hauptschleife - läuft endlos
   while(TRUE) {
      // Auf Events warten (geht in Low-Power-Mode wenn keine Events)
      Event_wait();

      // Event-Handler: Button 2 wurde gedrückt
      if (Event_tst(EVENT_BTN2)) {
         Event_clr(EVENT_BTN2);  // Event als behandelt markieren

         // Zum nächsten Muster wechseln (1->2->3->4->5->6->1...)
         if (++cnt GT MUSTER6) {
            cnt = MUSTER1;  // Nach MUSTER6 wieder zu MUSTER1
         }

         // Neues Blinkmuster aktivieren
         set_blink_muster(cnt);
      }

      // Event-Handler: Button 1 wurde gedrückt
      if (Event_tst(EVENT_BTN1)) {
         Event_clr(EVENT_BTN1);  // Event als behandelt markieren

         // LED1 toggeln (umschalten zwischen AN und AUS)
         TGLBIT(P2OUT, BIT7);
      }

      // Event-Handler: Event-Fehler aufgetreten
      // (Event wurde gesetzt bevor das vorherige behandelt wurde)
      if (Event_err()) {
         // Fehlerbehandlung:
         TA0CTL = 0;              // Timer A0 stoppen
         SETBIT(P1OUT, BIT2);     // LED2 einschalten (Fehler-Anzeige)
         // System bleibt in diesem Zustand (Fehler persistent)
      }
   }
}



