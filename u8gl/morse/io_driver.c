/*
 * io_driver.c
 *
 * Created: 2016-10-06 00:43:32
 *  Author: peter
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "io_driver.h";

/***
function to get the select button value
*/

char key_pressed;

int Select_buttion()
{
	int ret =0;
	if((PORTH.IN & PIN7_bm) ==0 )
	{
		ret =1;
	}
	return ret;
}
/***
function to get the select button1
*/
int buttion_one()
{
	int ret =0;
	if((PORTH.IN & PIN2_bm) ==0 )
	{
		ret =1;
	}
	
	return ret;
}
/***
function to get the select button2
*/
int buttion_two()
{
	int ret =0;
	if((PORTH.IN & PIN3_bm) ==0 )
	{
		ret =1;
	}
	
	return ret;
}
/***
function to get the select button3
*/
int buttion_three()
{
	int ret =0;
	if((PORTH.IN & PIN4_bm) ==0 )
	{
		ret =1;
	}
	
	return ret;
}
/***
function to get the select four
*/
int buttion_four()
{
	int ret =0;
	if((PORTH.IN & PIN5_bm) ==0 )
	{
		ret =1;
	}
	
	return ret;
}

/***
function to get the select five
*/
int buttion_five()
{
	int ret =0;
	if((PORTH.IN & PIN7_bm) ==0 )
	{
		ret =1;
	}
	
	return ret;
}



/***
function to get the ptt from radio i aux port
*/
int Radio_ptt()
{
	int ret =0;
	if((PORTA.IN & PIN7_bm) ==0 )
	{
		ret =1;
	}
	
	return ret;
} 
/************************************************************************/
/* fuction to set pullup on buttons           */
/************************************************************************/
void setup_buttons()
{
	PORTH.PIN4CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN2_bm;
	PORTH.PIN4CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN3_bm;
	PORTH.PIN4CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN4_bm;
	PORTH.PIN4CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN5_bm;
	PORTH.PIN4CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN6_bm;
	PORTH.PIN4CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN7_bm;

}

void button_test()
{
    if (buttion_five() == 1)
	{
		printf("button five");
	}

	if (buttion_four() == 1)
	{
		printf("button four");
		printf("\n");
	}
	if (buttion_three() == 1)
	{
		printf("button three");
		printf("\n");
	}
	if (buttion_two() == 1)
	{
		printf("button two");
		printf("\n");
	}
	if (buttion_one() == 1)
	{
		printf("button one");
		printf("\n");
	}
	if (Select_buttion() ==1)
	{
		printf("button select");
		printf("\n");
	}
	
	
	
}

// alert led toogle functions
void toogle_alert()
{
	PORTK.OUTSET = PIN7_bm;
}
void clear_alert()
{
	PORTK.OUTCLR = PIN7_bm;
}
/** Generates pezoo beep */
void beep(int duration )
{
	/** tone 1 khz*/
	for (int i =0;i< duration; i++)
	{
		Max_write( 0,  1);
		_delay_ms(1);
		Max_write( 0, 0);
		_delay_ms(1);
	}

}
