/*
 * declarations.h
 *
 * Created: 04.09.2012 10:57:30
 *  Author: gautegam
 */


#ifndef DECLARATIONS_H_
#define DECLARATIONS_H_

/****************************************************************************
  Global definitions and offsets
****************************************************************************/


#define ADC_BASE 0x1400
#define OLED_COMMAND_BASE 0x1000
#define OLED_DATA_BASE 0x1200
#define FRAME_BASE 0x1800
#define FRAME_COLUMN_SIZE 128
#define FRAME_ROW_SIZE 64

#define BYTE uint8_t
#define TRUE 1
#define FALSE 0
#define NOT_INIT 99

#define SETBIT(ADDRESS,BIT)  (ADDRESS |= (1<<BIT))              // Remember that PORTX is for output and PINX is for reading
#define CLEARBIT(ADDRESS,BIT)  (ADDRESS &= ~(1<<BIT))   // Remember that PORTX is for output and PINX is for reading
#define TOGGLEBIT(ADDRESS,BIT)  (ADDRESS ^= (1<<BIT))   // Remember that PORTX is for output and PINX is for reading
#define CHECKBIT(ADDRESS,BIT)  (ADDRESS & (1<<BIT))             // Remember that PORTX is for output and PINX is for reading

/****************************************************************************
  Definitions concerning states in main and in the menu
****************************************************************************/
//STATES IN MAIN
#define STATE_WELCOME 0
#define STATE_LEAVE_WELCOME 1
#define STATE_MENU 2
#define STATE_GAME 3
#define STATE_HELP 4
#define STATE_SETTINGS 5
#define STATE_LEDS 6
#define STATE_HIGHSCORE 7
#define STATE_MODE 8


//MENU STATES
#define MENU_GAME 3
#define MENU_HELP 4
#define MENU_SETTINGS 5

//SETTINGS STATES
#define SETTINGS_LEDS 6
#define SETTINGS_HIGHSCORE 7
#define SETTINGS_MODE 8


//MENU DIRECTIONS
#define NONE 0
#define UP 1
#define DOWN 2

/****************************************************************************
  Definitions concerning can
****************************************************************************/
//MESSAGE TYPES
#define POSITION 1
#define RESTART 2
#define LIFELOST 3
#define GAMESTART 4
#define SOLONOID 5
#define CAM_X_SET 6
#define CAM_Y_POS 7
#define TOGGLE_KNIGHTRIDER_SEQ 8
#define CHANGE_MODE 9

//CONTROLLER MODES FOR NODE 2´S MOTOR
#define MODE_PID 0
#define MODE_MANUAL 1
#define MODE_INIT 2

//CAN IDs
#define USBID 1
#define STKID 2
#define BREADID 3

/****************************************************************************
  Definitions concerning joystick
****************************************************************************/
#define CHANNEL_Y_JOY 1
#define CHANNEL_X_JOY 2
#define CHANNEL_RIGHT_SLIDER 4
#define CHANNEL_LEFT_SLIDER 3
#define AXIS_Y 1
#define AXIS_X 0

/****************************************************************************
  Definitions concerning oled
****************************************************************************/
#define FONT_SIZE 8
#define FONT_LEFT 0
#define FONT_CENTER 1

/****************************************************************************
  Global includes
****************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#endif /* DECLARATIONS_H_ */

