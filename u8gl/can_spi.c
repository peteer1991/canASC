/*
 * spi.c
 *
 * Created: 04.10.2012 09:20:42
 *  Author: peter
 */ 
#include "can/declarations.h"
//#include "spi.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>



		
void SPI_init()
{
	PORTD.DIRSET = 0xB0;  // configure MOSI, SS, CLK as outputs on PORTE
	PORTD.OUTSET =PIN4_bm;
	// enable SPI master mode, CLK/64 (@32MHz=>500KHz)
	SPID.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV64_gc;
	

}




void SPI_write(char cData)
{
	SPID.DATA = cData;
	while(!(SPID.STATUS & (1<<7)));
	_delay_us(10);
}


unsigned char SPI_read()	//Remember that to read something from the slave, the master must transmit a dummy
{
	SPI_write(0xff);
	return SPID.DATA;
}

unsigned char SPI_read_write(char cData)	//Remember that to read something from the slave, the master must transmit a dummy
{
	SPI_write(0xff);
	return SPID.DATA;
}

void spi_writeread(unsigned char in, unsigned char *out)
{
    SPID.DATA = in;
    loop_until_bit_is_set(SPID.STATUS, SPI_IF_bp);
    
    *out = SPID.DATA;
}
