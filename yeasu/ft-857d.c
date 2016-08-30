/*
 * ft_857d.c
 *
 * Created: 2016-08-31 00:31:47
 *  Author: peter
 */ 

#include "FT-857D.h"
#define BYTE uint8_t
#include <avr/interrupt.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <ctype.h>
#include <inttypes.h>
#include "../Controller_def.h"

int count_active =0; // kollar om logprogramet är på
int scan_for =0;
int count_active_temp =0;
extern radio rs232radio;
extern void sendChar_d();
int controller_tx =0;
int stand_alone_transmit =0;

int number_of_readed_byte =0;
int number_of_transmitted_byte =0;
char yeasu_read[5];
int number_of_transmitted_byte_pc =0;
char pc_read[5];
int cat_message_type =0;


/** settignup interup timmer tcc0 */
void setup_timmer()
{
	// read timmer for radio timeout
	TCC0.CNT = 0;// Zeroise count
	TCC0.PER = 9000; //Period
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc; //Divider
	TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc; //Liow level interrupt
	TCC0.INTFLAGS = 0x01; // clear any initial interrupt flags
	TCC0.CTRLB = TC_WGMODE_NORMAL_gc; // Normal operation
}

/** Send the paket to radio for freqvency and mode */
void send_get_freq()
{

	BYTE serial[5] ={0xFF,0xFF,0xFF,0xFF,0x03};
	

	for (int i =0; i<8;i++)
	{
		if (i <5)
		{
			sendChar_d(serial[i]);
		}
		else
		{
			sendChar_d(0xFF);
		}
		
	}
	controller_tx=1;


}
/** Send the paket to radio for status of settings */
void send_get_status()
{
	BYTE serial[5] ={0xFF,0xFF,0xFF,0xFF,0xF7};


	for (int i =0; i<8;i++)
	{
		if (i <5)
		{
			sendChar_d(serial[i]);
		}
		
	}
	controller_tx=1;

}
/** Send the paket to radio for RX settings */
void send_get_rxstatus()
{
	BYTE serial[5] ={0xFF,0xFF,0xFF,0xFF,0xE7};


	for (int i =0; i<8;i++)
	{
		if (i <5)
		{
			sendChar_d(serial[i]);
		}
		
	}
	controller_tx=1;

}
/** Send the paket to radio for BB pakket */
void send_BB()
{
	BYTE serial[5] ={0xFF,0xFF,0xFF,0xFF,0x80};


	for (int i =0; i<8;i++)
	{
		if (i <5)
		{
			sendChar_d(serial[i]);
		}
		
	}
	controller_tx=1;

}

// GPL
// unsigned long from hamlib work
long from_bcd_be(char bcd_data[], int bcd_len)
{
	int i;
	long f = 0;

	for (i=0; i < bcd_len/2; i++) {
		f *= 10;
		f += bcd_data[i]>>4;
		f *= 10;
		f += bcd_data[i] & 0x0f;
	}
	if (bcd_len&1) {
		f *= 10;
		f += bcd_data[bcd_len/2]>>4;
	}
	return f;
}

// GPL
// taken from hamlib work
unsigned char * to_bcd_be( unsigned char bcd_data[], unsigned long  freq, unsigned bcd_len)
{
	int i;
	unsigned char a;

	if (bcd_len&1) {
		bcd_data[bcd_len/2] &= 0x0f;
		bcd_data[bcd_len/2] |= (freq%10)<<4;
		/* NB: low nibble is left uncleared */
		freq /= 10;
	}
	for (i=(bcd_len/2)-1; i >= 0; i--) {
		a = freq%10;
		freq /= 10;
		a |= (freq%10)<<4;
		freq /= 10;
		bcd_data[i] = a;
	}
	return bcd_data;
}


ISR(USARTD0_RXC_vect)
{
	char test1a =USARTD0_DATA;
	yeasu_read[number_of_readed_byte] = test1a;
	number_of_readed_byte++;
	
	if(number_of_readed_byte >=5)
	{
		number_of_readed_byte =0;
		// copy the message to the from yeasy yx
		switch(cat_message_type)
		{
			case CAT_READ_FREQ_MODE:
			// Decode the freqvensy from radio
			rs232radio.freqvensy = from_bcd_be(yeasu_read, 8);
			rs232radio.radio_mode = yeasu_read[4];
			break;
			
			default:
			break;
			
		}

	}
	
	USARTC0_DATA = test1a;

}


ISR(USARTC0_RXC_vect)
{
	char rx_data = USARTC0_DATA;

	pc_read[number_of_transmitted_byte_pc] = rx_data;
	
	number_of_transmitted_byte_pc++;
	
	if(number_of_transmitted_byte_pc >4)
	{
		number_of_transmitted_byte_pc =0;
		switch(pc_read[4])
		{
			case CAT_RX_FREQ_CMD:
			cat_message_type= CAT_READ_FREQ_MODE;
			
			number_of_readed_byte =0;
			break;
			default:
			cat_message_type =0;
			number_of_readed_byte =0;
			break;
		}
		
	}
	
	USARTD0_DATA = rx_data;
	
	count_active++;

	
}
// skriver kommandon till radion för att få ut data då logprogramet ej är på
ISR(TCC0_OVF_vect)
{
	// koden skickar ut data till radion då
	// logprogrammet inte gör det
	
	if (count_active_temp == count_active )
	{
		number_of_readed_byte=0;
		switch(scan_for)
		{
			case 0:
				send_get_freq();
				cat_message_type = CAT_READ_FREQ_MODE;
				scan_for++;
				break;
			case 1:
				send_get_status();
				cat_message_type = CAT_READ_STATUS;
				scan_for++;
			break;
			case 2:
				stand_alone_transmit=1;
				send_get_rxstatus();
				cat_message_type = CAT_READ_RXSTATUS;
				scan_for++;
			break;
			default:
				scan_for=0;
				controller_tx=0;
			break;
			
		}
		
	}
	
	count_active_temp =count_active;

}
