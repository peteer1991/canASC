/*
 * mcp2515.c
 *
 * Created: 04.10.2012 09:23:03
 *  Author: siriholt
 */
#include "mcp2515.h"
#include "declarations.h"
#include "MCP2515define.h"

extern void SPI_write();
extern void SPI_init();
extern unsigned char SPI_read(); 
/****************************************************************************
Call this function to set up the mcp2515 to its initial standby state.
****************************************************************************/
BYTE mcp2515_init(){    
	 
        SPI_init();		
        mcp2515_reset();       
        return 0;
}

/****************************************************************************
Call this function to read from a specified address.
****************************************************************************/
BYTE mcp2515_read(BYTE address){
 
        // Chip select
       PORTD.OUTCLR =PIN4_bm;
		BYTE result;
        // Write instruction
        SPI_write(0x03);
        // Where to read from
        SPI_write(address);
        result = SPI_read();
        PORTD.OUTSET =PIN4_bm;
    
        return result;
}

/****************************************************************************
Call this function to read from a the receive buffer.
****************************************************************************/
BYTE mcp2515_read_rx_buffer(BYTE number){
        BYTE result;
        // Chip select
     
		PORTD.OUTCLR =PIN4_bm;
        // Write instruction
        switch (number){
                case 0:
                        SPI_write(MCP_READ_RX0);
                        break;
                case 1:
                        SPI_write(MCP_READ_RX1);
                        break;
        }
        result = SPI_read();
        // Chip unselect
		PORTD.OUTSET =PIN4_bm;
               
        return result;
}

/****************************************************************************
Call this function to write a byte to a specified address.
****************************************************************************/
void mcp2515_write(BYTE data, BYTE address){
       
        // Chip select
        PORTD.OUTCLR =PIN4_bm;
        // Write instruction
        SPI_write(MCP_WRITE);
        // Where to write to
        SPI_write(address);
        // What to send
        SPI_write(data);
        // Chip unselect
        PORTD.OUTSET =PIN4_bm;
       
}

uint8_t mcp2515_read_register(uint8_t address)
{
	uint8_t data;

	PORTD.OUTCLR =PIN4_bm;
	SPI_write(MCP2515_SPI_READ);     //READ Instruction
	SPI_write(address);
	data=SPI_read();
	PORTD.OUTSET =PIN4_bm;

	return data;
}

void mcp2515_write_register(BYTE data, BYTE address){
	
	// Chip select
	PORTD.OUTCLR =PIN4_bm;
	// Write instruction
	SPI_write(MCP2515_SPI_WRITE);
	// Where to write to
	SPI_write(address);
	// What to send
	SPI_write(data);
	// Chip unselect
	PORTD.OUTSET =PIN4_bm;
	
}

//At power up, MCP2515 buffers are not truly empty. There is random data in the registers
//This loads buffers with zeros to prevent incorrect data to be sent.
void CAN_MCP2515_clearRxBuffers()
{
	PORTD.OUTCLR =PIN4_bm;
	SPI_write(MCP2515_SPI_WRITE);
	SPI_write(MCP2515_RXB0SIDH);
	for (uint8_t i = 0; i < 13; i++)
	{
		SPI_write(0x00);
	}
	PORTD.OUTSET =PIN4_bm;
	PORTD.OUTCLR =PIN4_bm;;
	SPI_write (MCP2515_SPI_WRITE);
	SPI_write(MCP2515_RXB1SIDH);
	for (uint8_t i = 0; i < 13; i++)
	{
		SPI_write(0x00);
	}
		PORTD.OUTSET =PIN4_bm;
}

// This loads buffers with zeros to prevent incorrect data to be sent.
// Note: If RTS is sent to a buffer that has all zeros it will still send a message with all zeros.
void CAN_MCP2515_clearTxBuffers()
{
	PORTD.OUTCLR =PIN4_bm;;
	SPI_write(MCP2515_SPI_WRITE);
	SPI_write(MCP2515_TXB0SIDH);
	for (uint8_t i = 0; i < 13; i++)
	{
		SPI_write(0x00);
	}
	PORTD.OUTSET =PIN4_bm;
	PORTD.OUTCLR =PIN4_bm;;
	SPI_write (MCP2515_SPI_WRITE);
	SPI_write (MCP2515_TXB1SIDH);
	for (uint8_t i = 0; i < 13; i++)
	{
	  SPI_write(0x00);
	}
	PORTD.OUTSET =PIN4_bm;
	PORTD.OUTCLR =PIN4_bm;	
	
	SPI_write (MCP2515_SPI_WRITE);
	SPI_write (MCP2515_TXB2SIDH);
	for (uint8_t i = 0; i < 13; i++)
	{
		SPI_write(0x00);
	}
	PORTD.OUTSET =PIN4_bm;
}


/****************************************************************************
Call this function to send from a specified buffer.
****************************************************************************/
void mcp2515_request_to_send(BYTE rts_buffer)
{
        // Chip select
        PORTD.OUTCLR =PIN4_bm;
        // Write instruction
        switch(rts_buffer){
                case MCP_RTS_TX0:
                        SPI_write(MCP_RTS_TX0);
                        break;
                case MCP_RTS_TX1:
                        SPI_write(MCP_RTS_TX1);
                        break;
                case MCP_RTS_TX2:
                        SPI_write(MCP_RTS_TX2);
                        break;
                case MCP_RTS_ALL:
                        SPI_write(MCP_RTS_ALL);
                        break;
        }
        // Chip Unselect
        PORTD.OUTSET =PIN4_bm;
}

/****************************************************************************
Call this function to bit modify a specified buffer
****************************************************************************/
void mcp2515_bit_modify(BYTE address, uint8_t data, BYTE mask)
{
        // Chip Select
        PORTD.OUTCLR =PIN4_bm;
        // Write instruction
        SPI_write(MCP_BITMOD);
        // Write address
        SPI_write(address);             // Where we will change bits
		// Write mask
        SPI_write(mask);                // Which bits are allowed to change
		// Write data
        SPI_write(data);                // Which bits we will change to
        // Chip Unselect
        PORTD.OUTSET =PIN4_bm;
}

/****************************************************************************
Call this function to reset the mcp2515
****************************************************************************/
void mcp2515_reset()
{
        // Chip select
        PORTD.OUTCLR =PIN4_bm;
        // Write instruction
        SPI_write(MCP2515_SPI_RESET);
        // Chip unselect
        PORTD.OUTSET =PIN4_bm;
}

/****************************************************************************
Call this function to read the status on the mcp2515
****************************************************************************/
BYTE mcp2515_read_status()
{
        BYTE value;
        // Chip Select
        PORTD.OUTCLR =PIN4_bm;
        // Write Instruction
        SPI_write(MCP_READ_STATUS);
		value = SPI_read();
        // Chip unselect
        PORTD.OUTSET =PIN4_bm;
        return value;
}

