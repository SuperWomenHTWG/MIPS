/*
 * TA0.h - Timer A0 Header
 *
 * Dieses Modul verwaltet Timer A0 für LED-Blinkmuster.
 * Es stehen 6 verschiedene Blinkmuster zur Verfügung.
 */

#ifndef TA0_H_
#define TA0_H_

#include "..\base.h"

// Blinkmuster-Definitionen (0-5)
#define MUSTER1  (0)  // Blinkmuster 1
#define MUSTER2  (1)  // Blinkmuster 2
#define MUSTER3  (2)  // Blinkmuster 3
#define MUSTER4  (3)  // Blinkmuster 4
#define MUSTER5  (4)  // Blinkmuster 5
#define MUSTER6  (5)  // Blinkmuster 6

// Initialisiert Timer A0 und startet ihn
EXTERN inline Void TA0_init(Void);

// Wählt das aktive Blinkmuster aus
// Parameter: Musternummer (MUSTER1 bis MUSTER6)
EXTERN Void set_blink_muster(UInt);

#endif /* TA0_H_ */
