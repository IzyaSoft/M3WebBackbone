/*
 * ------------------------------------------------------------------------------------------------
 *   o o o o       Fachhochschule fuer Technik und Informatik TI
 *         :....o  Fachbereich Elektro- und Kommunikationstechnik
 * ------------------------------------------------------------------------------------------------
 *
 * Projekt ..........: Ausbildungskit Mikrocontroller mit dem LPC1768
 * Dateiname ........: kit1768.c
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
 * Modulbeschreibung : Support Functions for the ARM Cortex LPC1768 Kit.
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

#include <stdlib.h>

#include "kit1768.h"

#include "lpc_types.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_gpio.h"

/*
 * ------------------------------------------------------------------------------------------------
 * Definitionen
 * ------------------------------------------------------------------------------------------------
 */

const UINT8 SEGMENT_TABLE[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

/*
 * ------------------------------------------------------------------------------------------------
 * Globale Variablen
 * ------------------------------------------------------------------------------------------------
 */

volatile unsigned long SysTickCounter;		/* SysTick Counter		*/

/*
 * ------------------------------------------------------------------------------------------------
 * Function: SysTick_Handler
 *
 * Description: SysTick Interrupt Handler 10 ms
 *
 * Parameters: Keine
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void SysTick_Handler(void)
 {
  if (SysTickCounter != 0)
   SysTickCounter--;
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_10msDelay
 *
 * Description: Delay. Base is 10 ms.
 *
 * Parameters: Delay Wert in ms
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_10msDelay(UINT32 delayValue)
 {
  SysTickCounter = delayValue;
  while (SysTickCounter != 0);
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Init
 *
 * Description: Inititilisieren KIT1768
 *
 * Parameters: Keine
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_Init(void)
 {
  GPIO_SetDir(TASTEN_PORT0, TASTE_0, INPUT);
  GPIO_SetDir(TASTEN_PORT1, TASTE_5, INPUT);
  GPIO_SetDir(TASTEN_PORT2, TASTEN_P2, INPUT);

  GPIO_SetDir(SCHALTER_PORT0, SCHALTER_P0, INPUT);
  GPIO_SetDir(SCHALTER_PORT1, SCHALTER_P1, INPUT);

  GPIO_SetDir(LED_PORT, LED_ALL, OUTPUT);
  GPIO_SetDir(LED_PORT, LED_ENABLE, OUTPUT);
  GPIO_SetValue(LED_PORT, LED_ENABLE);

  GPIO_SetDir(SEG_EN_PORT2, SEG0_ENABLE, OUTPUT);
  GPIO_SetDir(SEG_EN_PORT2, SEG1_ENABLE, OUTPUT);
  GPIO_SetDir(SEG_EN_PORT2, SEG2_ENABLE, OUTPUT);
  GPIO_SetDir(SEG_EN_PORT0, SEG3_ENABLE, OUTPUT);
  GPIO_ClearValue(SEG_EN_PORT2, SEG0_ENABLE | SEG1_ENABLE | SEG2_ENABLE);
  GPIO_ClearValue(SEG_EN_PORT0, SEG3_ENABLE);
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Led_Enable
 *
 * Description: Enable the LEDs auf dem IO-Board.
 *
 * Parameters: 0 -> Disable, 1 -> Enable
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_Led_Enable(BOOLEAN enable)
 {
  if (enable)
   GPIO_SetValue(LED_PORT, LED_ENABLE);
  else
   GPIO_ClearValue(LED_PORT, LED_ENABLE);
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Led_Ein
 *
 * Description: Einschalten einer beliebigen LED auf dem IO-Board.
 *
 * Parameters: Nummer der LED die eingeschaltet wird.
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_Led_Ein(UINT8 LedNummer)
 {
  switch (LedNummer)
   {
    case BIT_0:
     GPIO_SetValue(LED_PORT, LED_0);
    break;

    case BIT_1:
     GPIO_SetValue(LED_PORT, LED_1);
    break;

    case BIT_2:
     GPIO_SetValue(LED_PORT, LED_2);
    break;

    case BIT_3:
     GPIO_SetValue(LED_PORT, LED_3);
    break;

    case BIT_4:
     GPIO_SetValue(LED_PORT, LED_4);
    break;

    case BIT_5:
     GPIO_SetValue(LED_PORT, LED_5);
    break;

    case BIT_6:
     GPIO_SetValue(LED_PORT, LED_6);
    break;

    case BIT_7:
     GPIO_SetValue(LED_PORT, LED_7);
    break;
   }
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Alle_Led_Ein
 *
 * Description: Einschalten aller LED auf dem IO-Board.
 *
 * Parameters: Eine Eins im Wert ledValue schaltet die LED ein eine Null schaltet sie aus.
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_Alle_Led_Ein(UINT8 ledValue)
 {
  UINT8 i, mask;

  mask = 1;
  for (i=0; i<8; i++)
   {
	if (ledValue & mask)
	 KIT1768_Led_Ein(i);
	else
	 KIT1768_Led_Aus(i);
	mask = mask << 1;
   }
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Led_Aus
 *
 * Description: Ausschalten einer beliebigen LED auf dem IO-Board.
 *
 * Parameters: Nummer der LED die ausgeschaltet werden soll.
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_Led_Aus(UINT8 LedNummer)
 {
  switch (LedNummer)
   {
    case BIT_0:
     GPIO_ClearValue(LED_PORT, LED_0);
    break;

    case BIT_1:
     GPIO_ClearValue(LED_PORT, LED_1);
    break;

    case BIT_2:
     GPIO_ClearValue(LED_PORT, LED_2);
    break;

    case BIT_3:
     GPIO_ClearValue(LED_PORT, LED_3);
    break;

    case BIT_4:
     GPIO_ClearValue(LED_PORT, LED_4);
    break;

    case BIT_5:
     GPIO_ClearValue(LED_PORT, LED_5);
    break;

    case BIT_6:
     GPIO_ClearValue(LED_PORT, LED_6);
    break;

    case BIT_7:
     GPIO_ClearValue(LED_PORT, LED_7);
    break;
   }
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Alle_Led_Aus
 *
 * Description: Ausschalten aller LED auf dem IO-Board.
 *
 * Parameters: Eine Eins schaltet die LED aus.
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_Alle_Led_Aus(UINT8 ledValue)
 {
  UINT8 i, mask;

  mask = 1;
  for (i=0; i<8; i++)
   {
	if (ledValue & mask)
	 KIT1768_Led_Aus(i);
	mask = mask << 1;
   }
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Segment_Ein
 *
 * Description: Einschalten einer Ziffer der 7-Segment Anzeige auf dem IO-Board.
 *
 * Parameters: Ziffer für 7-Segment 0..9
 *             7-Segment Nummer 0..3
 *
 * Returned Value: Kein
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

void KIT1768_Segment_Ein(UINT8 segValue, UINT8 nummer)
 {
  KIT1768_Led_Enable(FALSCH);
  if ((segValue < 10) && (segValue >= 0))
   KIT1768_Alle_Led_Ein(SEGMENT_TABLE[segValue]);
  switch (nummer)
   {
    case 0:
     GPIO_SetValue(SEG_EN_PORT2, SEG0_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT2, SEG1_ENABLE | SEG2_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT0, SEG3_ENABLE);
    break;

    case 1:
     GPIO_SetValue(SEG_EN_PORT2, SEG1_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT2, SEG0_ENABLE | SEG2_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT0, SEG3_ENABLE);
    break;

    case 2:
     GPIO_SetValue(SEG_EN_PORT2, SEG2_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT2, SEG0_ENABLE | SEG1_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT0, SEG3_ENABLE);
    break;

    case 3:
     GPIO_SetValue(SEG_EN_PORT0, SEG3_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT2, SEG0_ENABLE | SEG1_ENABLE | SEG2_ENABLE);
    break;

    default:
     GPIO_ClearValue(SEG_EN_PORT2, SEG0_ENABLE | SEG1_ENABLE | SEG2_ENABLE);
     GPIO_ClearValue(SEG_EN_PORT0, SEG3_ENABLE);
   }
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Lese_Taste
 *
 * Description: Lesen einer beliebigen Taste auf dem IO-Board.
 *
 * Parameters: Nummer der Taste die gelesen werden soll.
 *
 * Returned Value: WAHR -> Taste ist gedrueckt.
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

BOOLEAN KIT1768_Lese_Taste(UINT8 TastenNummer)
 {
  switch (TastenNummer)
   {
    case BIT_0:
     if ((GPIO_ReadValue(TASTEN_PORT0) & TASTE_0) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_1:
     if ((GPIO_ReadValue(TASTEN_PORT2) & TASTE_1) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_2:
     if ((GPIO_ReadValue(TASTEN_PORT2) & TASTE_2) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_3:
     if ((GPIO_ReadValue(TASTEN_PORT2) & TASTE_3) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_4:
     if ((GPIO_ReadValue(TASTEN_PORT1) & TASTE_4) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_5:
     if ((GPIO_ReadValue(TASTEN_PORT2) & TASTE_5) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_6:
    if ((GPIO_ReadValue(TASTEN_PORT2) & TASTE_6) != 0)
     return WAHR;
    else
     return FALSCH;
    break;

    case BIT_7:
     if ((GPIO_ReadValue(TASTEN_PORT2) & TASTE_7) != 0)
      return WAHR;
     else
      return FALSCH;
    break;
   }
  return FALSCH;
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Lese_Alle_Tasten
 *
 * Description: Lesen aller Tasten auf dem IO-Board.
 *
 * Parameters: Keine.
 *
 * Returned Value: Tastenwerte
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

UINT8 KIT1768_Lese_Alle_Tasten(void)
 {
  UINT8 i, mask, status;

  status = 0;
  mask = 1;
  for (i=0; i<8; i++)
   {
	if (KIT1768_Lese_Taste(i))
	 status = status | mask;
	mask = mask << 1;
   }
  return status;
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Lese_Schalter
 *
 * Description: Lesen eines  beliebigen Schalter auf dem IO-Board.
 *
 * Parameters: Nummer des Schalters der gelesen werden soll.
 *
 * Returned Value: WAHR -> Schalter ist gesetzt.
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

BOOLEAN KIT1768_Lese_Schalter(UINT8 SchalterNummer)
 {
  switch (SchalterNummer)
   {
    case BIT_0:
     if ((GPIO_ReadValue(SCHALTER_PORT0) & SCHALTER_0) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_1:
     if ((GPIO_ReadValue(SCHALTER_PORT0) & SCHALTER_1) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_2:
     if ((GPIO_ReadValue(SCHALTER_PORT0) & SCHALTER_2) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_3:
     if ((GPIO_ReadValue(SCHALTER_PORT0) & SCHALTER_3) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_4:
     if ((GPIO_ReadValue(SCHALTER_PORT1) & SCHALTER_4) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_5:
     if ((GPIO_ReadValue(SCHALTER_PORT1) & SCHALTER_5) != 0)
      return WAHR;
     else
      return FALSCH;
    break;

    case BIT_6:
    if ((GPIO_ReadValue(SCHALTER_PORT1) & SCHALTER_6) != 0)
     return WAHR;
    else
     return FALSCH;
    break;

    case BIT_7:
     if ((GPIO_ReadValue(SCHALTER_PORT0) & SCHALTER_7) != 0)
      return WAHR;
     else
      return FALSCH;
    break;
   }
  return FALSCH;
 }

/*
 * ------------------------------------------------------------------------------------------------
 * Function: KIT1768_Lese_Alle_Schalter
 *
 * Description: Lesen aller Schalter auf dem IO-Board.
 *
 * Parameters: Keine.
 *
 * Returned Value: Wert der Schalterstellung.
 *
 * Assumptions:
 *
 * ------------------------------------------------------------------------------------------------
 */

UINT8 KIT1768_Lese_Alle_Schalter(void)
 {
  UINT8 i, mask, status;

  status = 0;
  mask = 1;
  for (i=0; i<8; i++)
   {
	if (KIT1768_Lese_Schalter(i))
	 status = status | mask;
	mask = mask << 1;
   }
  return status;
 }
