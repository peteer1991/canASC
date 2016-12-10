/* Project name project configuration file	*/

#ifndef _project_config_h_
#define _project_config_h_

/* define MCU family and configuration fuses if any */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>



#include "GenericMacros.h"
#include "GenericTypeDefs.h"
#include "USARTio.h"
#include "MAX3421E.h"
#include "cli.h"
#include "USB.h"
#include "usb_ch9.h"
#include "transfer.h"
#include "HID.h"


//#define CLOCK	3	//milliseconds 			




#define set_BPNT_0 	BPNT_0 = 1;
#define	clr_BPNT_0	BPNT_0 = 0;
#define tgl_BPNT_0	set_BPNT_0; clr_BPNT_0

/* Async serial settings					*/
/* USART transmit and receive buffer sizes	*/
/* must be a power of 2						*/
#define USART_TX_BUFSIZE 16
#define USART_TX_BUF_MASK ( USART_TX_BUFSIZE - 1 )
#if (USART_TX_BUFSIZE & USART_TX_BUF_MASK)
	#error USART Tx buffer size is not a power of 2
#endif

#define USART_RX_BUFSIZE 16
#define USART_RX_BUF_MASK ( USART_RX_BUFSIZE - 1 )
#if (USART_RX_BUFSIZE & USART_RX_BUF_MASK)
	#error USART Rx buffer size is not a power of 2
#endif

/* USB constants */
/* time in milliseconds */
#define USB_SETTLE_TIME 					200         //USB settle after reset
#define USB_XFER_TIMEOUT					5000        //USB transfer timeout

#define USB_NAK_LIMIT 200
#define USB_RETRY_LIMIT 3



#endif // _project_config_h