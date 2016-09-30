/*
 * ------------------------------------------------------------------------------------------------
 *   o o o o       Fachhochschule fuer Technik und Informatik TI
 *         :....o  Fachbereich Elektro- und Kommunikationstechnik
 * ------------------------------------------------------------------------------------------------
 *
 * Projekt ..........: Ausbildungskit Mikrocontroller mit dem LPC1768
 * Dateiname ........: kit1768.h
 * Pfad .............: ${file_path}
 * Autor ............: Martin Aebersold (AOM1)
 * KIT ..............: BFH LPC1768-Kit
 * Plattform ........: CoIDE Version 1.2.1
 * Toolchain ........: gcc version 4.5.1 (Sourcery G++ Lite 2010.09-51)
 * Company...........: Berne University of Applied Sciences TI
 * Internet..........: www.hti.bfh.ch
 *
 * ------------------------------------------------------------------------------------------------
 *
 * Modulbeschreibung : Header Datei für das BFH LPC1768 Ausbildungskit.
 *
 * ------------------------------------------------------------------------------------------------
 * Aenderungen und Korrekturen:
 *     Version | Datum      | Kuerzel | Beschreibung
 *     --------|------------|---------|------------------------------------------------------------
 *     1.0     | 21.01.2011 |  AOM1   | Erstellung der Datei
 * ------------------------------------------------------------------------------------------------
 */

/*
 *-------------------------------------------------------------------------------------------------
 * Header-Dateien
 *-------------------------------------------------------------------------------------------------
 */

#ifndef _KIT1768_H_
#define _KIT1768_H_

#include <stdlib.h>

#include "../lpc17xx_lib/include/lpc17xx_clkpwr.h"
#include "../lpc17xx_lib/include/lpc17xx_gpio.h"

/*
 * ------------------------------------------------------------------------------------------------
 * Datentypen
 * ------------------------------------------------------------------------------------------------
 */

typedef signed char			INT8;
typedef unsigned char		UINT8;
typedef signed short		INT16;
typedef unsigned short		UINT16;
typedef signed long			INT32;
typedef unsigned long		UINT32;
typedef signed long long	INT64;
typedef unsigned long long	UINT64;
typedef float				FLOAT32;
typedef double				FLOAT64;

typedef enum {FALSCH = 0, WAHR = !FALSCH} BOOLEAN;

/*
 * ------------------------------------------------------------------------------------------------
 * Makros
 * ------------------------------------------------------------------------------------------------
 */


/*
 * ------------------------------------------------------------------------------------------------
 * Definitionen
 * ------------------------------------------------------------------------------------------------
 */

#define SystemCoreClock 72000000L

#define OUTPUT		1
#define INPUT		0

/* Data Bit-Positionen	*/
#define	BIT_0		0
#define	BIT_1		1
#define	BIT_2		2
#define	BIT_3		3
#define	BIT_4		4
#define	BIT_5		5
#define	BIT_6		6
#define	BIT_7		7

/* Bit-Positionen der Tasten	*/
#define	TASTE_0		(1<<26)
#define	TASTE_1		(1<<2)
#define	TASTE_2		(1<<3)
#define	TASTE_3		(1<<4)
#define	TASTE_4		(1<<29)
#define	TASTE_5		(1<<0)
#define	TASTE_6		(1<<1)
#define	TASTE_7		(1<<12)

#define TASTEN_P2	TASTE_1 | TASTE_2 | TASTE_3 | TASTE_5 | TASTE_6 | TASTE_7

/* Bit-Positionen der Schalter	*/
#define	SCHALTER_0	(1<<4)
#define	SCHALTER_1	(1<<5)
#define	SCHALTER_2	(1<<15)
#define	SCHALTER_3	(1<<25)
#define	SCHALTER_4	(1<<25)
#define	SCHALTER_5	(1<<26)
#define	SCHALTER_6	(1<<28)
#define	SCHALTER_7	(1<<10)

#define SCHALTER_P0	SCHALTER_0 | SCHALTER_1 | SCHALTER_2 | SCHALTER_3 | SCHALTER_7
#define SCHALTER_P1 SCHALTER_4 | SCHALTER_5 | SCHALTER_6

/* Bit-Positionen der LEDs		*/
#define	LED_0		(1<<16)
#define	LED_1		(1<<17)
#define	LED_2		(1<<18)
#define	LED_3		(1<<19)
#define	LED_4		(1<<20)
#define	LED_5		(1<<21)
#define	LED_6		(1<<22)
#define	LED_7		(1<<23)

#define LED_ALL		LED_7 | LED_6 | LED_5 | LED_4 | LED_3 | LED_2 | LED_1 | LED_0

/* Bit-Position LED Freigabe	*/
#define	LED_ENABLE	(1<<11)

/* Bit-Positionen der 7-Segment Freigaben	*/
#define SEG0_ENABLE	(1<<5)
#define SEG1_ENABLE	(1<<6)
#define SEG2_ENABLE	(1<<11)
#define SEG3_ENABLE	(1<<27)

#define SEG_ENABLE_ALL	SEG3_ENABLE | SEG2_ENABLE | SEG1_ENABLE | SEG0_ENABLE

#define	SEG_EN_PORT0	0
#define	SEG_EN_PORT2	2

#define LED_PORT		0

#define TASTEN_PORT0	0
#define TASTEN_PORT1	1
#define TASTEN_PORT2	2

#define SCHALTER_PORT0	0
#define SCHALTER_PORT1	1
#define SCHALTER_PORT2	2

/*
 * ------------------------------------------------------------------------------------------------
 * Funktions-Prototypen
 * ------------------------------------------------------------------------------------------------
 */

void 	KIT1768_Init(void);
void 	KIT1768_10msDelay(UINT32 delayValue);
void 	KIT1768_Led_Ein(UINT8 LedNummer);
void 	KIT1768_Alle_Led_Ein(UINT8 ledValue);
void 	KIT1768_Led_Aus(UINT8 LedNummer);
void 	KIT1768_Alle_Led_Aus(UINT8 ledValue);
void 	KIT1768_Segment_Ein(UINT8 segValue, UINT8 nummer);
BOOLEAN KIT1768_Lese_Taste(UINT8 TastenNummer);
UINT8 	KIT1768_Lese_Alle_Tasten(void);
BOOLEAN KIT1768_Lese_Schalter(UINT8 SchalterNummer);
UINT8 	KIT1768_Lese_Alle_Schalter(void);

#endif
