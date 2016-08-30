/*
 * can.c
 *
 * Created: 04.10.2012 09:22:00
 *  Author: siriholt
 */

#include "can.h"
#include "declarations.h"
#include "mcp2515.h"
#include "MCP2515define.h"

BYTE solonoid_sent = FALSE;
BYTE toggle_sent = FALSE;


/****************************************************************************
Call this function to set up the CAN to its initial standby state.
****************************************************************************/
void CAN_init(){
        mcp2515_init();
        //Set receive interrupt

       // set speed to 
	    //mcp2515_bit_modify(MCP2515_CANCTRL,MCP2515_MODE_CONFIG , MCP2515_REQOPn);
		CAN_MCP2515_clearTxBuffers();
		CAN_MCP2515_clearRxBuffers();
		

		CAN_MCP2515_setBitrate(250000);
		
		mcp2515_bit_modify(MCP_CANINTE, 0x3, MCP_RX_INT);
        //Set control register to turn off mask filter and receive any msgs
        mcp2515_bit_modify(MCP_RXB0CTRL, 0x60, 0xFF);
		//Set mode normal
        mcp2515_bit_modify(MCP_CANCTRL, MODE_NORMAL, MODE_MASK);
		
				
		printf("\n Can setings %i \t",mcp2515_read_register(MCP2515_CNF1));
		printf("%i\t",mcp2515_read_register(MCP2515_CNF2));
		printf("%i\n",mcp2515_read_register(MCP2515_CNF3));

        CAN_int_flag = 0;
		
}

void CAN_MCP2515_setBitrate(uint32_t bitrate)
{
  uint8_t CNF1 = 0; 
  uint8_t CNF2 = 0;
  uint8_t CNF3 = 0;
 if (bitrate == 5000)
  {
    CNF1 = 0x3F;
    CNF2 = 0xFF;
    CNF3 = 0x87;
  }
  else if (bitrate == 10000)
  {
    CNF1 = 0x1F;
    CNF2 = 0xFF;
    CNF3 = 0x87;
  }
  else if (bitrate == 20000)
  {
    CNF1 = 0x0F;
    CNF2 = 0xFF;
    CNF3 = 0x87;
  }
  else if (bitrate == 31025)
  {
    CNF1 = 0x0F;
    CNF2 = 0xF1;
    CNF3 = 0x85;
  }
  else if (bitrate == 40000)
  {
    CNF1 = 0x07;
    CNF2 = 0xFF;
    CNF3 = 0x87;
  }
  else if (bitrate == 50000)
  {
    CNF1 = 0x07;
    CNF2 = 0xFA;
    CNF3 = 0x87;
  }
  else if (bitrate == 80000)
  {
    CNF1 = 0x03;
    CNF2 = 0xFF;
    CNF3 = 0x87;
  }
  else if (bitrate == 100000)
  {
    CNF1 = 0x03;
    CNF2 = 0xFA;
    CNF3 = 0x87;
  }
  else if (bitrate == 125000)
  {
    CNF1 = 0x03;
    CNF2 = 0xF0;
    CNF3 = 0x86;
  }
  else if (bitrate == 200000)
  {
    CNF1 = 0x01;
    CNF2 = 0xFA;
    CNF3 = 0x87;
  }
  else if (bitrate == 250000)
  {
    CNF1 = 0x41;
    CNF2 = 0xF1;
    CNF3 = 0x85;
  }
  else if (bitrate == 500000)
  {
    CNF1 = 0x00;
    CNF2 = 0xF0;
    CNF3 = 0x86;
  }
  else if (bitrate == 1000000)
  {
    CNF1 = 0x00;
    CNF2 = 0xD0;
    CNF3 = 0x82;
  }

// 	mcp2515_write_register(MCP2515_CNF1, CNF1);//Write config address 1
// 	mcp2515_write_register(MCP2515_CNF2, CNF2);//Write config address 2
// 	mcp2515_write_register(MCP2515_CNF3, CNF3);//Write config address 3
	mcp2515_bit_modify(MCP2515_CNF1, CNF1,0xFF);//Write config address 1
	mcp2515_bit_modify(MCP2515_CNF2, CNF2,0xFF);//Write config address 2
	mcp2515_bit_modify(MCP2515_CNF3, CNF3,0xFF);//Write config address 3
}

/****************************************************************************
Call this function to send a message over the CAN bus
****************************************************************************/
void CAN_message_send(can_message_t* msg){
        mcp2515_write(msg->msg_id, MCP_TXB0SIDH);
		//mcp2515_write(msg->msg_id, MCP2515_TXB0EID0);
        mcp2515_bit_modify(MCP_TXB0DLC, msg->data_length, 0x0F);
        BYTE i;
        for(i = 0; i <  msg->data_length; i++){
                mcp2515_write(msg->data[i], MCP_TXB0D0 + i);
        }
        mcp2515_request_to_send(MCP_RTS_TX0);
}

/****************************************************************************
Call this function to receive a message over the CAN bus. Note that
the interrupt is cleared by bit modification.
****************************************************************************/
void CAN_data_receive(can_message_t* msg) {
        BYTE i;
        for(i = 0; i < 8; i++){
                msg->data[i] = 0;
        }
        msg->msg_id = mcp2515_read(MCP_RXB0SIDH);
		
		
		char val;
		val = mcp2515_read(MCP2515_RXB0EID0);
		if (msg->msg_id ==0)
		{
			 msg->msg_id  = (msg->msg_id  << 8) | val;
		}
      
		
        msg->data_length = mcp2515_read(MCP_RXB0DLC);
        for(i = 0; i < msg->data_length; i++){
                msg->data[i] = mcp2515_read(MCP_RXB0D0 + i);
        }
        //Clear interrupt
        mcp2515_bit_modify(MCP_CANINTF, 0x00, 0x01);
        //CAN_int_flag = 0;
}


/****************************************************************************
This is an interrupt service routine used to notify users that a CAN
message is received. It sets the interrupt flag used in main.
****************************************************************************/
/*
ISR(INT1_vect)
{ // CAN receive interrupt
        CAN_int_flag = 1;
}

*/