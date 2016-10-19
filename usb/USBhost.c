/* pic18f26k20-based MAX3421E based Lightweight USB host */
#include <util/delay.h>
#define _LwUSBhost_c_
#include <stdio.h>
#include <string.h>

#include "project_config.h"

extern HID_DEVICE hid_device;
extern DEV_RECORD devtable[];

/* Global variables  */
volatile DWORD uptime = 0;                      //system uptime. Gets updated every millisecond

volatile BYTE USART_Tx_buf [ USART_RX_BUFSIZE ];
volatile BYTE USART_Tx_head = 0;
volatile BYTE USART_Tx_tail = 0;
volatile BYTE USART_Rx_buf [ USART_RX_BUFSIZE ];
volatile BYTE USART_Rx_head = 0;
volatile BYTE USART_Rx_tail = 0;
extern BYTE usb_task_state;

/* Prototypes   */
void highPriorityISR(void);
void lowPriorityISR(void);
void Board_init( void );
extern int meny_selected;
extern char key_pressed;
void update_uptime()
{
	uptime=uptime+60;
}


/* ISRs */
/*
#pragma code high_vector=0x08
void interruptAtHighVector(void)
{
    _asm GOTO highPriorityISR _endasm
}
#pragma code
#pragma code low_vector=0x18
void interruptAtLowVector(void)
{
    _asm GOTO lowPriorityISR _endasm
}
#pragma code
#pragma interruptlow lowPriorityISR
*/
/*
void lowPriorityISR(void)
{
 BYTE data,tmphead,tmptail;           //USART vars
 USART handler start 
    if(PIR1bits.RCIF) {
        data = RCREG;                                                   // read the received data 
        tmphead = ( USART_Rx_head + 1 ) & USART_RX_BUF_MASK;            // calculate buffer index 
        USART_Rx_head = tmphead;                                        // store new index 
        if ( tmphead == USART_Rx_tail ) {
        // ERROR! Receive buffer overflow 
        }
        USART_Rx_buf[ tmphead ] = data;                                 // store received data in buffer 
  }
    //? need to check for Tx IF
    if(TXSTAbits.TRMT) {    
                                                                        // check if all data is transmitted  
        if ( USART_Tx_head != USART_Tx_tail ) {                                     
            tmptail = ( USART_Tx_tail + 1 ) & USART_TX_BUF_MASK;        // calculate buffer index  
            USART_Tx_tail = tmptail;                                    // store new index  
            TXREG = USART_Tx_buf[ tmptail ];                            // start transmition  
        }
        else {
            PIE1bits.TXIE = 0;         // disable TX interrupt  
        }
    } 
 USART handler end */
//}

/* Timer1 interrupt handler */
void highPriorityISR(void)
{
    uptime++;                       
	//IR2bits.CCP2IF = 0;
}

void main_setup( void )
{
	SPI_init_usb();
	MAX3421E_init();
	USB_init();
	
}

void main_usb ( void )
{
	MAX3421E_Task();
	USB_Task();   
	if (usb_task_state == 64)
	{
		testKbd( 1 );
	}
	
}

void Board_init( void )
{

}

char crlf[5] ="\r\n";
BYTE HIDtoa( BOOT_KBD_REPORT* buf, BYTE index );
BOOL prevCodeComp( BYTE data, BOOT_KBD_REPORT* buf );

/* keyboard communication demo              */
/* only basic functions/keys are supported  */
void testKbd( BYTE addr )
{
	char i;
	BYTE rcode;
	char tmpbyte;
	//char* byteptr;
	BOOT_KBD_REPORT kbdbuf;
	BOOT_KBD_REPORT localbuf;

	rcode = XferGetIdle( addr, 0, hid_device.interface, 0, &tmpbyte );
	rcode = XferGetProto( addr, 0, hid_device.interface, &tmpbyte );
	if( rcode ) {   //error handling
		printf("\r\nGetProto Error. Error code ");
		printf( "%i",rcode );
	}
	//delay = uptime + 5;
	//while( uptime < delay );    //wait polling interval
	rcode = kbdPoll( &kbdbuf );

	//        byteptr = ( char *)&kbdbuf;
	//        send_string( crlf );
	//        for( i = 0; i < 8; i++ ) {
	//            send_hexbyte( *byteptr );
	//            byteptr++;
	//        }
	//send_string( crlf );
	for( i = 0; i < 1; i++ ) {
		if( kbdbuf.keycode[ i ] == 0 ) {        //empty position means it and all subsequent positions are empty
			break;
		}
		if( prevCodeComp( kbdbuf.keycode[ i ], &localbuf ) == FALSE ) {
			//                send_hexbyte( kbdbuf.keycode[ i ] );
			char print_char= HIDtoa( &kbdbuf,i );
			if(print_char != 0x07)
				printf("%c", print_char);
			//                send_string( crlf );
			switch(print_char)
			{
				case '+':
					meny_selected++;
					break;
				case '-':
					meny_selected--;
					break;
				default:
					break;
					
			}
			key_pressed = print_char;
		}
		memcpy(( char* )&localbuf, ( const  char* )&kbdbuf, sizeof( BOOT_KBD_REPORT )); 
	}

}

BOOL prevCodeComp( BYTE data, BOOT_KBD_REPORT* buf )
{
	BYTE i;
	for( i = 0; i < 6; i++ ) {
		if( buf->keycode[ i ] == data ) {
			return( TRUE );
		}
	}
	return( FALSE );
}

BYTE HIDtoa( BOOT_KBD_REPORT* buf, BYTE index  )
{
	BYTE HIDcode = buf->keycode[ index ];
	//BYTE AsciiVal;
	//BYTE ShiftkeyStatus = 0;
	/* symbols a-z,A-Z */
	if( HIDcode >= 0x04 && HIDcode <= 0x1d ) {
		if( buf->mod.LShift || buf->mod.RShift ) {                          //uppercase
			return( HIDcode + 0x3d );
		}
		if( buf->mod.LCtrl || buf->mod.RCtrl ) {                            //Ctrl
			return( HIDcode - 3 );
		}
		return( HIDcode + 0x5d );                             //lowercase
	}

	/** Numbpad keys */
	switch( HIDcode ) {
		case( 0x59 ): //1
			return( '1' ); 
			break;
		case( 0x5a ):      
			return( '2' ); 
			break;
		case( 0x5b ):
			return( '3' );
			break;
		case( 0x5c ):
			return( '4' );
			break;
		case( 0x5d ):
			return( '5' );
			break;
		case( 0x5e ):
			return( '6' );
			break;		
		case( 0x5f ):
			return( '7' );
			break;
		case( 0x60 ):
			return( '8' );
			break;		
		case( 0x61 ):
			return( '9' );
			break;
		case( 0x2b ):
			return( 't' );
			break;	
		case( 0x54 ):
			return( '/' );
			break;
		case( 0x55 ):
			return( '*' );
			break;
		case( 0x2a ):
			return( 'b' );
			break;
		case( 0x56 ):
			return( '-' );
			break;		
		case( 0x57 ):
			return( '+' );
			break;
		default:
			break;
	}
	
	
	
	
	return( 0x07 );         //Bell
}
