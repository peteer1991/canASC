/*
 * max3421e.c
 *
 * Created: 2016-10-07 23:56:59
 *  Author: peter
 */ 
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/delay.h>
#include <stdio.h>
#include "max3421e.h"
#include "Max3421e_constants.h"
#include "usb.h"
#include "kdb.h"


static byte vbusState;

void USBH_SpiInit(void) 
{
	PORTC.DIR = 0xB0;  // configure MOSI, SS, CLK as outputs on PORTF
	PORTF.DIR = 0xB0;  // configure MOSI, SS, CLK as outputs on PORTF
	// enable SPI master mode, CLK/64 (@32MHz=>500KHz)
	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV64_gc;
}

/**
 * Write data to a register over SPIF
 * @param reg uint8_t  - the register you want to write to
 * @param data uint8_t - the register you want to read from
 * @return void
 */
void USBH_SpiWrite(uint8_t reg, uint8_t data)
{
	PORTF.OUTCLR = PIN6_bm; 		// lower ss line to start of transfer
	SPIC.DATA = reg;      			// initiate write
	while(!(SPIC.STATUS & (1<<7))); // wait for transfer complete
	SPIC.DATA = data;      			// initiate write
	while(!(SPIC.STATUS & (1<<7))); // wait for transfer complete
	PORTF.OUTSET = PIN6_bm;  		// raise ss line to end of transfer
}




/**
 * Read a byte from a register.
 * @param reg uint8_t  - the register you want to write to
 * @return char - date read from SPIF.DATA
 */
char USBH_SpiRead(uint8_t reg)
{
  USBH_SpiWrite(reg, 0x00);	// write 0x00 and read back results clocked into data buffer
  return SPIC.DATA;
}




/* Single host register write   */
void regWr( byte reg, byte val)
{
	USBH_SpiWrite(reg,val);
	return;
}

byte regRd( byte reg )
{
	return USBH_SpiRead(reg);
}


/* probe bus to determine device presense and speed and switch host to this speed */
void busprobe( void )
{
	byte bus_sample;
	bus_sample = regRd( rHRSL );            //Get J,K status
	bus_sample &= ( bmJSTATUS|bmKSTATUS );      //zero the rest of the byte
	switch( bus_sample ) {                          //start full-speed or low-speed host
		case( bmJSTATUS ):
		if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
			regWr( rMODE, MODE_FS_HOST );       //start full-speed host
			vbusState = FSHOST;
		}
		else {
			regWr( rMODE, MODE_LS_HOST);        //start low-speed host
			vbusState = LSHOST;
		}
		break;
		case( bmKSTATUS ):
		if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
			regWr( rMODE, MODE_LS_HOST );       //start low-speed host
			vbusState = LSHOST;
		}
		else {
			regWr( rMODE, MODE_FS_HOST );       //start full-speed host
			vbusState = FSHOST;
		}
		break;
		case( bmSE1 ):              //illegal state
		vbusState = SE1;
		break;
		case( bmSE0 ):              //disconnected state
		regWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ);
		vbusState = SE0;
		break;
	}//end switch( bus_sample )
}


void USBH_powerOn()
{
	/* Configure full-duplex SPI, interrupt pulse   */
	regWr( rPINCTL,( bmFDUPSPI + bmINTLEVEL + bmGPXB ));    //Full-duplex SPI, level interrupt, GPX
	/*
	if( reset() == 0 ) {                                //stop/start the oscillator
		printfln("Error: OSCOKIRQ failed to assert");
	}
	*/
	/* configure host operation */
	regWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ );      // set pull-downs, Host, Separate GPIN IRQ on GPX
	regWr( rHIEN, bmCONDETIE|bmFRAMEIE );                                             //connection detection
	/* check if device is connected */
	regWr( rHCTL,bmSAMPLEBUS );                                             // sample USB bus
	while(!(regRd( rHCTL ) & bmSAMPLEBUS ));                                //wait for sample operation to finish
	busprobe();                                                             //check if anything is connected
	regWr( rHIRQ, bmCONDETIRQ );                                            //clear connection detect interrupt
	regWr( rCPUCTL, 0x01 );                                                 //enable interrupt pin
}
byte Task( void )
{
	byte rcode = 0;
	//printf("Vbus state: ");
	//printfln( vbusState, HEX );
	
	if((PORTF.IN & PIN7_bm) ==0 )
	{
		rcode = IntHandler();
	}
// 	pinvalue = digitalRead( MAX_GPX );
// 	if( pinvalue == LOW ) {
// 		GpxHandler();
// 	}
	//    usbSM();                                //USB state machine
	return( rcode );
}

byte IntHandler()
{
	byte HIRQ;
	byte HIRQ_sendback = 0x00;
	HIRQ = regRd( rHIRQ );                  //determine interrupt source
	//if( HIRQ & bmFRAMEIRQ ) {               //->1ms SOF interrupt handler
	//    HIRQ_sendback |= bmFRAMEIRQ;
	//}//end FRAMEIRQ handling
	if( HIRQ & bmCONDETIRQ ) {
		busprobe();
		HIRQ_sendback |= bmCONDETIRQ;
	}
	/* End HIRQ interrupts handling, clear serviced IRQs    */
	regWr( rHIRQ, HIRQ_sendback );
	return( HIRQ_sendback );
}


/* Initialize keyboard */
void kbd_init( void )
{
	byte rcode = 0;  //return code
	/**/
	/* Initialize data structures */
	//*ep_record[ 0 ] =getDevTableEntry( 0,0);  //copy endpoint 0 parameters
	ep_record[ 1 ].MaxPktSize = EP_MAXPKTSIZE;
	ep_record[ 1 ].Interval  = EP_POLL;
	ep_record[ 1 ].sndToggle = bmSNDTOG0;
	ep_record[ 1 ].rcvToggle = bmRCVTOG0;
	setDevTableEntry( 1, ep_record );              //plug kbd.endpoint parameters to devtable
	/* Configure device */
	rcode = setConf( KBD_ADDR, 0, 1 );
	if( rcode ) {
		printf("Error attempting to configure keyboard. Return code :");
		printf( rcode );
		while(1);  //stop
	}
	/* Set boot protocol */
	rcode = setProto( KBD_ADDR, 0, 0, 0 );
	if( rcode ) {
		printf("Error attempting to configure boot protocol. Return code :");
		printf( rcode );
		while( 1 );  //stop
	}

	printf("Keyboard initialized");
}

/* HID to ASCII converter. Takes HID keyboard scancode, returns ASCII code */
byte HIDtoA( byte HIDbyte, byte mod )
{
	/* upper row of the keyboard, numbers and special symbols */
	if( HIDbyte >= 0x1e && HIDbyte <= 0x27 ) {
		if(( mod & SHIFT ) || numLock ) {    //shift key pressed
			switch( HIDbyte ) {
				case BANG:  return( 0x21 );
				case AT:    return( 0x40 );
				case POUND: return( 0x23 );
				case DOLLAR: return( 0x24 );
				case PERCENT: return( 0x25 );
				case CAP: return( 0x5e );
				case AND: return( 0x26 );
				case STAR: return( 0x2a );
				case OPENBKT: return( 0x28 );
				case CLOSEBKT: return( 0x29 );
			}//switch( HIDbyte...
		}
		else {                  //numbers
			if( HIDbyte == 0x27 ) {  //zero
				return( 0x30 );
			}
			else {
				return( HIDbyte + 0x13 );
			}
		}//numbers
	}//if( HIDbyte >= 0x1e && HIDbyte <= 0x27
	/**/
	/* number pad. Arrows are not supported */
	if(( HIDbyte >= 0x59 && HIDbyte <= 0x61 ) && ( numLock == 1 )) {  // numbers 1-9
		return( HIDbyte - 0x28 );
	}
	if(( HIDbyte == 0x62 ) && ( numLock == 1 )) {                      //zero
		return( 0x30 );
	}
	/* Letters a-z */
	if( HIDbyte >= 0x04 && HIDbyte <= 0x1d ) {
		if((( capsLock == 1 ) && ( mod & SHIFT ) == 0 ) || (( capsLock == 0 ) && ( mod & SHIFT ))) {  //upper case
			return( HIDbyte + 0x3d );
		}
		else {  //lower case
			return( HIDbyte + 0x5d );
		}
	}//if( HIDbyte >= 0x04 && HIDbyte <= 0x1d...
	/* Other special symbols */
	if( HIDbyte >= 0x2c && HIDbyte <= 0x38 ) {
		switch( HIDbyte ) {
			case SPACE: return( 0x20 );
			case HYPHEN:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x2d );
			}
			else {
				return( 0x5f );
			}
			case EQUAL:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x3d );
			}
			else {
				return( 0x2b );
			}
			case SQBKTOPEN:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x5b );
			}
			else {
				return( 0x7b );
			}
			case SQBKTCLOSE:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x5d );
			}
			else {
				return( 0x7d );
			}
			case BACKSLASH:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x5c );
			}
			else {
				return( 0x7c );
			}
			case SEMICOLON:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x3b );
			}
			else {
				return( 0x3a );
			}
			case INVCOMMA:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x27 );
			}
			else {
				return( 0x22 );
			}
			case TILDE:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x60 );
			}
			else {
				return( 0x7e );
			}
			case COMMA:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x2c );
			}
			else {
				return( 0x3c );
			}
			case PERIOD:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x2e );
			}
			else {
				return( 0x3e );
			}
			case FRONTSLASH:
			if(( mod & SHIFT ) == 0 ) {
				return( 0x2f );
			}
			else {
				return( 0x3f );
			}
			default:
			break;
		}//switch( HIDbyte..
	}//if( HIDbye >= 0x2d && HIDbyte <= 0x38..
	return( 0 );
}


void Usb_Task()
{
	//Task();

   
	/*
	if( getUsbTaskState() == USB_STATE_CONFIGURING ) {  //wait for addressing state
		
		//setUsbTaskState( USB_STATE_RUNNING );
	}
	if( getUsbTaskState() == USB_STATE_RUNNING ) {  //poll the keyboard
	
	}
	*/
	
}
