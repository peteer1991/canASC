/*
 * menus.c
 *
 * Created: 2015-06-22 23:17:16
 *  Author: peter
 */ 
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>

#include "menus_def.h"
void MAIN_INFO_F();
void MAIN_AMPS_F();
void MAIN_ROTORS_F();
void MAIN_SETTINGS_F();
void MAIN_ANTENNAS_F();

void SETINGS_RADIO_F();
void SETINGS_PC_F();
void SETINGS_CAN_F();
void SETINGS_BACK_F();
#include "u8g/u8g.h"
extern u8g_t u8g;
extern int rad_test;
extern int meny_selected;
// in main
extern void toogle_alert();
extern int Select_buttion() ;
extern int buttion_one();
extern int buttion_two();

void Get_number_of_units();
extern int count_rot();
extern int count_sw();
extern void draw_angel_circle();
extern int count_amp();
extern void clear_alert();







void execute_function(int i)
{
	//men->Trigger_function[i](men);
}

void error_screen(char * text)
{
	toogle_alert();
		u8g_FirstPage(&u8g);
    do
    {
	  u8g_SetFont(&u8g, u8g_font_6x10);
	  u8g_DrawRFrame(&u8g, 1, 1, 126, 62, 2);

	  u8g_DrawStr(&u8g, 6, 20, text);
	  
 
    } while ( u8g_NextPage(&u8g) );	
	_delay_ms(1000);
	clear_alert();
	
}
int rad_test;
void GOTO_Direction()
{
	int meny_saved =meny_selected;
	meny_selected = rad_test;

		
		// 360 deg fix
		if(meny_selected > 360)
			 meny_selected =0;
		if(meny_selected <0)
			 meny_selected =360;
		
		u8g_FirstPage(&u8g);
		do
		{
		 
		  u8g_SetFont(&u8g, u8g_font_6x10);
		  draw_angel_circle(rad_test);
				  
 
		} while ( u8g_NextPage(&u8g) );
		
	
	
	meny_selected =meny_saved; 
	
}

void MAIN_ROTORS_F()
{
	if(count_rot() == 0)
		error_screen("No ROTORS FOUND!");
	
}
void MAIN_ANTENNAS_F()
{
	if(count_sw() == 0)
		error_screen("NO ANTENNA SWITCHES FOUND!");
}

void MAIN_AMPS_F()
{
	if(count_amp() == 0)
		error_screen("NO Amplifier found!");
}
void MAIN_SETTINGS_F()
{
	while (Select_buttion() ==1);

}
void MAIN_INFO_F()
{
	printf("%i \r",5);
	Get_number_of_units();
}

void SETINGS_RADIO_F()
{
	printf("%i \r",1);	
}
void SETINGS_PC_F()
{
	printf("%i \r",2);
}
void SETINGS_CAN_F()
{
	printf("%i \r",3);
}
void SETINGS_BACK_F()
{

}

