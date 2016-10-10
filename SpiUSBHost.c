/*
 * SpiUSBHost.c
 *
 *  Created on: May 13, 2009
 *      Author: Vito Vecchio
 *
 *
 *
 *
 */
#include <avr/interrupt.h>
#include <avr/io.h>


#define USBSEL  IO1CLR = GPIO_IO_P20
#define USBUSEL IO1SET = GPIO_IO_P20
#define BYTE char;



unsigned portLONG SPIInit( void )
{
    TxCounter = 0;

    U32 spsr;


    PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P04_MASK | PCB_PINSEL0_P05_MASK | PCB_PINSEL0_P06_MASK )) | (PCB_PINSEL0_P04_SCK0 | PCB_PINSEL0_P05_MISO0 | PCB_PINSEL0_P06_MOSI0 );
    GPIO1_FIODIR |= GPIO_IO_P20;
    GPIO1_FIOSET  = GPIO_IO_P20;
    spsr = SPI_SPSR;
    S0SPCCR = 0x08;
#if INTERRUPT_MODE
  if ( install_irq( SPI0_INT, (void *)SPI0Handler ) == FALSE )
    {
	return (FALSE);
    }
    /* 8 bit, CPOL=CPHA=0, master mode, MSB first, interrupt enabled */
    S0SPCR = SPI0_SPIE | SPI0_MSTR;
#else
    S0SPCR = SPI0_MSTR;/* 8 bit, CPOL=CPHA=0, master mode, MSB first */
#endif
    return( TRUE );
}


void Host_SPI_Write(BYTE reg, BYTE data)
{
	USBSEL;

	S0SPDR = reg|0x02;
	while ((S0SPSR & SPIF) == 0x00);
	S0SPDR = data;
	while ((S0SPSR & SPIF) == 0x00);

	USBUSEL;

}


unsigned short Host_SPI_Read(BYTE reg)
{

	BYTE data;

	USBSEL;

	S0SPDR = reg;
	while((S0SPSR & SPIF) == 0x00 );

	S0SPDR = 0x00;
	while((S0SPSR & SPIF) == 0x00 );

	USBUSEL;

	return S0SPDR;

}



void Host_Write_N_Bytes(BYTE reg, int Bytes_num, BYTE *pdata)
{
    USBSEL;

    int i;
    S0SPDR = reg | 0x02;

    while ((S0SPSR & SPIF) == 0x00);

    for(i=0; i<Bytes_num; i++)

    {
    	S0SPDR = *pdata;
    	while ((S0SPSR & SPIF) == 0x00);
    	pdata++;
    }

    USBUSEL;
}



void Host_Read_N_Bytes(BYTE reg, int Bytes_num, BYTE *prdata)
{
    USBSEL;

    int i;
    S0SPDR = reg;

    while ((S0SPSR & SPIF) == 0x00);

    for(i=0; i<Bytes_num; i++)

    {
    	S0SPDR = 0x00;
    	while ((S0SPSR & SPIF) == 0x00);
    	*prdata = S0SPDR ;
    	aspetta_ms(10);
    	prdata++;

    }

    USBUSEL;
}


void Host_wr_HostReg(BYTE reg, BYTE val)	// write a host register
{
USBSEL;

S0SPDR = reg |0x03;			// command byte. 0x02 = write bit, 0x01 is ACKSTAT bit
while ((S0SPSR & SPIF) == 0x00);
S0SPDR = val;// data byte into the FIFO
while ((S0SPSR & SPIF) == 0x00); // hang until BUSY bit goes low

USBUSEL;

}
