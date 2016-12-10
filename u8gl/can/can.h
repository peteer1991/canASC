/*
 * can.h
 *
 * Created: 04.10.2012 09:22:10
 *  Author: siriholt
 */


#ifndef CAN_H_
#define CAN_H_

#include "declarations.h"
#include "mcp2515.h"


/****************************************************************************
  Global definitions
****************************************************************************/

typedef struct{
        BYTE msg_id;
        BYTE data_length;
        BYTE data[8];
}can_message_t;

BYTE CAN_int_flag;

/****************************************************************************
  Function definitions
****************************************************************************/
void CAN_init();
void CAN_message_send(can_message_t* msg);
void CAN_data_receive(can_message_t* msg);
void CAN_send_joystick(BYTE switch3, BYTE movement);
void CAN_send_solonoid(BYTE switch3);
void CAN_send_toggle_knightrider(BYTE switch0);
void CAN_send_switch_mode_manual(BYTE switch2);
void CAN_send_switch_mode_pid(BYTE switch3);
void CAN_MCP2515_setBitrate();
void CAN_MCP2515_clearTxBuffers();
void CAN_MCP2515_clearRxBuffers();
uint8_t mcp2515_read_register();

typedef struct __attribute__((__packed__))
{
  uint32_t id      : 29;  // if (extended == CAN_RECESSIVE) { extended ID } else { standard ID }
  uint8_t valid    : 1;   // To avoid passing garbage frames around
  uint8_t rtr      : 1;   // Remote Transmission Request Bit (RTR)
  uint8_t extended : 1;   // Identifier Extension Bit (IDE)
  uint32_t fid;           // family ID
  uint8_t priority : 4;   // Priority but only important for TX frames and then only for special uses.
  uint8_t length   : 4;   // Data Length
  uint16_t timeout;       // milliseconds, zero will disable waiting
  uint8_t data[8];                        // Message data
} CAN_Frame;              // suffix of '_t' is reserved by POSIX for future use


#endif /* CAN_H_ */

