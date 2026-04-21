/*
 * base.h - Grundlegende Definitionen und Makros
 *
 * Diese Header-Datei enthält allgemeine Makros, Typdefinitionen
 * und Hilfsfunktionen für das gesamte Projekt
 */

#ifndef _BASE_H
#define _BASE_H

// Makro zur Speicherung der aktuellen Datei und Zeilennummer (für Debugging)
#define _WHERE_ __FILE__,__LINE__

/* ASSERT - Makro zur Laufzeitüberprüfung von Bedingungen */
#ifdef NOOS
// Wenn NOOS definiert ist (No OS), ist ASSERT deaktiviert
#define ASSERT(a)
#else
// #include <stdio.h>
void urks(void);
// ASSERT überprüft die Bedingung 'a' und beendet das Programm bei Fehler
#define ASSERT(a) if (!(a)) {urks();fprints("Assertion failed at %d in %s\n",__LINE__,__FILE__); exit(1);}
#endif


// Lesbare Vergleichsoperatoren (zur Verbesserung der Codelesbarkeit)
#define EQ    ==  // Equal (gleich)
#define NE    !=  // Not Equal (ungleich)
#define GE    >=  // Greater or Equal (größer oder gleich)
#define GT    >   // Greater Than (größer als)
#define LE    <=  // Less or Equal (kleiner oder gleich)
#define LT    <   // Less Than (kleiner als)

// Logische Operatoren (zur Verbesserung der Codelesbarkeit)
#define NOT   !   // Logisches NICHT
#define AND   &&  // Logisches UND
#define OR    ||  // Logisches ODER

// Bitweise Operatoren (zur Verbesserung der Codelesbarkeit)
#define BAND  &   // Bitweises UND
#define BOR   |   // Bitweises ODER
#define BNOT  ~   // Bitweises NICHT (Komplement)
#define XOR   ^   // Bitweises XOR (exklusives ODER)

/* Hilfsfunktionen für min, max und between
   (nicht in stdlib.h definiert!) */
#ifndef max
// Gibt das Maximum von zwei Werten zurück
#define max(a,b)  ((a) LT (b)) ? (b) : (a)
#endif

#ifndef min
// Gibt das Minimum von zwei Werten zurück
#define min(a,b)  ((a) GT (b)) ? (b) : (a)
#endif

#ifndef between
// Prüft ob 'b' zwischen 'a' und 'c' liegt (inklusive)
#define between(a,b,c)  ((a) LE (b)) AND ((b) LE (c))
#endif


/* Bit-Manipulations-Makros für Registeroperationen */
#define SETBIT(p,b) ((p)|=(b))   // Setzt Bit(s): p = p | b
#define CLRBIT(p,b) ((p)&= ~(b)) // Löscht Bit(s): p = p & ~b
#define TSTBIT(p,b) ((p)&(b))    // Testet Bit(s): gibt p & b zurück
#define TGLBIT(p,b) ((p)^=(b))   // Toggelt Bit(s): p = p ^ b (XOR)

// Modifikatoren für Funktions- und Variablen-Scope
#define Void    void     // Datentyp für "nichts"/"leer"
#define LOCAL   static   // Lokale (datei-interne) Sichtbarkeit
#define GLOBAL           // Globale Sichtbarkeit (leer, kein Modifier)
#define EXTERN  extern   // Externe Deklaration (in anderer Datei definiert)

/* Primitive Datentypen - Aliase für bessere Lesbarkeit */
typedef int             Int;      // Vorzeichenbehaftete Ganzzahl (meist 16 oder 32 Bit)
typedef unsigned int    UInt;     // Vorzeichenlose Ganzzahl
typedef long            Long;     // Vorzeichenbehaftete lange Ganzzahl (meist 32 Bit)
typedef unsigned long   ULong;    // Vorzeichenlose lange Ganzzahl
typedef short           Short;    // Vorzeichenbehaftete kurze Ganzzahl (meist 16 Bit)
typedef unsigned short  UShort;   // Vorzeichenlose kurze Ganzzahl
typedef char            Char;     // Einzelnes Zeichen (8 Bit)
typedef char *          String;   // Zeichenkette (Zeiger auf char)
typedef unsigned char   UChar;    // Vorzeichenloses Zeichen (0-255)
typedef double          Double;   // Fließkommazahl mit doppelter Genauigkeit
typedef float           Float;    // Fließkommazahl mit einfacher Genauigkeit

// Datentyp eines Funktionspointers (Zeiger auf eine Funktion ohne Parameter und Rückgabewert)
typedef Void (* VoidFunc)(Void);

// Bool-Typ: Behandlung von Konflikten mit X11/Xlib.h
#ifdef Bool /* Bool ist in <X11/Xlib.h> bereits definiert */
#undef Bool  // Vorherige Definition entfernen
#endif

#ifndef Bool
typedef int Bool;  // Boolescher Datentyp (0 = false, !0 = true)
#endif

// Wahrheitswerte
#ifndef FALSE
#define FALSE 0        // Falscher/unwahr Wert (0)
#endif
#ifndef TRUE
#define TRUE  (!FALSE) // Wahrer/wahr Wert (alles außer 0, hier: 1)
#endif

// NULL-Zeiger Konstante
#ifndef NULL
#define NULL 0  // Null-Zeiger (ungültige Adresse)
#endif

#endif  // Ende von _BASE_H
