/* MAX3421E low-level functions                             */
/* reading, writing registers, reset, host transfer, etc.   */
/* GPIN, GPOUT are as per tutorial, reassign if necessary   */
/* USB power on is GPOUT7, USB power overload is GPIN7      */
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define _MAX3421E_C_
#include "project_config.h"
#include "MAX3421E.h"

#define pinToRegister(P) ( pgm_read_byte( pin_to_register_PGM + (P) ) )
#define pinToBitMask(P) ( pgm_read_byte( pin_to_bit_mask_PGM + (P) ) )


const uint8_t PROGMEM   pin_to_register_PGM[] = {
	rIOPINS1, /* 0 */
	rIOPINS1,
	rIOPINS1,
	rIOPINS1,
	rIOPINS2, /* 4 */
	rIOPINS2,
	rIOPINS2,
	rIOPINS2,
};

const uint8_t PROGMEM   pin_to_bit_mask_PGM[] = {
	_BV(0), /* rIOPINS1 */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(0), /* rIOPINS2 */
	_BV(1),
	_BV(2),
	_BV(3),
};


/* variables and data structures */

/* External variables */

extern DWORD uptime;
extern BYTE usb_task_state;


/* Functions    */

/* SPI initialization */
/* this routine was borrowed from Microchip peripheral library. It's been rumored that they're going to stop including
    peripherals with new releases of C18 so I make a copy just in case.
    
        sync_mode:
                            SPI_FOSC_4          SPI Master mode, clock = FOSC/4
                            SPI_FOSC_16         SPI Master mode, clock = FOSC/16
                            SPI_FOSC_64         SPI Master mode, clock = FOSC/64
                            SPI_FOSC_TMR2       SPI Master mode, clock = TMR2 output/2
                            SLV_SSON            SPI Slave mode, /SS pin control enabled
                            SLV_SSOFF           SPI Slave mode, /SS pin control disabled
        bus_mode:
                            MODE_00             SPI bus Mode 0,0
                            MODE_01             SPI bus Mode 0,1
                            MODE_10             SPI bus Mode 1,0
                            MODE_11             SPI bus Mode 1,1
        smp_phase:
                            SMPEND              Input data sample at end of data out
                            SMPMID              Input data sample at middle of data out
*/
void SPI_init_usb( )
{

	PORTC.DIRSET = 0xB0;  // configure MOSI, SS, CLK as outputs on PORTE

	PORTF.DIRSET = PIN7_bm;  // configure MOSI, SS, CLK as outputs on PORTF
	
	PORTF.PIN6CTRL  =    PORT_OPC_PULLUP_gc;
	PORTF.DIRCLR    =    PIN6_bm;
	
	//PORTF.PIN6CTRL  =    PORT_OPC_PULLUP_gc;
	//PORTF.DIRSET = PIN6_bm;
	// enable SPI master mode, CLK/64 (@32MHz=>500KHz)
	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV64_gc;
//	Deselect_MAX3421E();
}

/* writes to SPI. BF is checked inside the procedure */
/* returns SSPBUF   */ 
BYTE SPI_wr( BYTE data )
{
	SPIC.DATA = data;      			// initiate write

	while(!(SPIC.STATUS & (1<<7))); // wait for transfer complete
	
	return SPIC.DATA;
}
void Select_MAX3421E()
{
		PORTF.OUTCLR = PIN7_bm; 		// lower ss line to start of transfer
}
void Deselect_MAX3421E()
{
		PORTF.OUTSET = PIN7_bm;  		// raise ss line to end of transfer
}

/* Single host register write   */
void MAXreg_wr(BYTE reg, BYTE val)
{
    Select_MAX3421E();
    SPI_wr ( reg + 2 ); //set WR bit and send register number
    SPI_wr ( val );
    Deselect_MAX3421E();
}
/* multiple-byte write */
/* returns a pointer to a memory position after last written */
BYTE* MAXbytes_wr( BYTE reg, BYTE nbytes, BYTE* data )
{
    Select_MAX3421E();    //assert SS
    SPI_wr ( reg +2  ); //set W/R bit and select register   
    while( nbytes ) {                
        SPI_wr( *data );    // send the next data byte
        data++;             // advance the pointer
        nbytes--;
    }
    Deselect_MAX3421E();  //deassert SS
    return( data );
}
/* Single host register read        */
BYTE MAXreg_rd( BYTE reg )    
{
	BYTE tmp;
    Select_MAX3421E();
    SPI_wr ( reg );         //send register number
    tmp = SPI_wr ( 0x00 );  //send empty byte, read register contents
    Deselect_MAX3421E(); 
    return (tmp);
}
/* multiple-bytes register read                             */
/* returns a pointer to a memory position after last read   */
char* MAXbytes_rd ( BYTE reg, BYTE nbytes, char* data )
{
    Select_MAX3421E();    //assert SS
    SPI_wr ( reg );     //send register number
    while( nbytes ) {
        *data = SPI_wr ( 0x00 );    //send empty byte, read register contents
        data++;
        nbytes--;
    }
    Deselect_MAX3421E();  //deassert SS
    return data;   
}
/* reset MAX3421E using chip reset bit. SPI configuration is not affected   */
BYTE MAX3421E_reset( void )
{
 BYTE tmp = 0;
    MAXreg_wr( rUSBCTL,bmCHIPRES );                     //Chip reset. This stops the oscillator
    MAXreg_wr( rUSBCTL,0x00 );                          //Remove the reset
    while(!(MAXreg_rd( rUSBIRQ ) & bmOSCOKIRQ )) {      //wait until the PLL stabilizes
        tmp++;                                          //timeout after 256 attempts
        if( tmp == 0 ) return( 0 );
		    
    }
	    return( 1 );
}
/* turn USB power on/off                                                */
/* ON pin of VBUS switch (MAX4793 or similar) is connected to GPOUT7    */
/* OVERLOAD pin of Vbus switch is connected to GPIN7                    */
/* OVERLOAD state low. NO OVERLOAD or VBUS OFF state high.              */
BOOL Vbus_power ( BOOL action )
{
    BYTE tmp = MAXreg_rd( rIOPINS2 );       //copy of IOPINS2
    if( action ) {                              //turn on by setting GPOUT7
        tmp |= bmGPOUT7;
    }
    else {                                      //turn off by clearing GPOUT7
        tmp &= ~bmGPOUT7;
    }
    MAXreg_wr( rIOPINS2,tmp );                              //send GPOUT7
    if( action )_delay_ms(60);
    if (!(MAXreg_rd( rIOPINS2 )&bmGPIN7)) return( FALSE );  // check if overload is present
    return( TRUE );                                             // power on/off successful                       
}

/* probe bus to determine device presense and speed */
void MAX_busprobe( void )
{
 BYTE bus_sample;
    
//  MAXreg_wr(rHCTL,bmSAMPLEBUS); 
    bus_sample = MAXreg_rd( rHRSL );            //Get J,K status
    bus_sample &= ( bmJSTATUS|bmKSTATUS );      //zero the rest of the byte

    switch( bus_sample ) {                          //start full-speed or low-speed host 
        case( bmJSTATUS ):
            /*kludgy*/
            if( usb_task_state != USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE ) { //bus reset causes connection detect interrupt
                if( (MAXreg_rd( rMODE ) & bmLOWSPEED ) == 0) {
                    MAXreg_wr( rMODE, MODE_FS_HOST );           //start full-speed host
                }
                else {
                    MAXreg_wr( rMODE, MODE_LS_HOST);    //start low-speed host
                }
                usb_task_state = ( USB_STATE_ATTACHED );    //signal usb state machine to start attachment sequence
            }
            break;
        case( bmKSTATUS ):
            if( usb_task_state != USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE ) { //bus reset causes connection detect interrupt
                if( (MAXreg_rd( rMODE ) & bmLOWSPEED ) == 0) {
                    MAXreg_wr( rMODE, MODE_LS_HOST );   //start low-speed host
                }
                else {
                    MAXreg_wr( rMODE, MODE_FS_HOST );               //start full-speed host
                }
                usb_task_state = ( USB_STATE_ATTACHED );    //signal usb state machine to start attachment sequence
            }
            break;
        case( bmSE1 ):              //illegal state
            usb_task_state = ( USB_DETACHED_SUBSTATE_ILLEGAL );
            break;
        case( bmSE0 ):              //disconnected state
            if( !(( usb_task_state & USB_STATE_MASK ) == USB_STATE_DETACHED ))          //if we came here from other than detached state
                usb_task_state = ( USB_DETACHED_SUBSTATE_INITIALIZE );  //clear device data structures
            else {  
              MAXreg_wr( rMODE, MODE_FS_HOST ); //start full-speed host
              usb_task_state = ( USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE );
            }
            break;
        }//end switch( bus_sample )
}
/* MAX3421E initialization after power-on   */
void MAX3421E_init( void )
{

    /* Configure full-duplex SPI, interrupt pulse   */
    MAXreg_wr( rPINCTL,(bmFDUPSPI+bmINTLEVEL+bmGPXB ));     //Full-duplex SPI, level interrupt, GPX
	if( MAX3421E_reset() == FALSE ) //stop/start the oscillator
	{                               
		printf("Error: OSCOKIRQ failed to assert");
	}
    /* configure power switch   */
    Vbus_power( OFF );                                      //turn Vbus power off
    MAXreg_wr( rGPINIEN, bmGPINIEN7 );                      //enable interrupt on GPIN7 (power switch overload flag)
    Vbus_power( ON  );
    /* configure host operation */
    MAXreg_wr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ );      // set pull-downs, SOF, Host, Separate GPIN IRQ on GPX
    //MAXreg_wr( rHIEN, bmFRAMEIE|bmCONDETIE|bmBUSEVENTIE );                      // enable SOF, connection detection, bus event IRQs
    MAXreg_wr( rHIEN, bmCONDETIE|bmFRAMEIE );                                     //connection detection
    /* HXFRDNIRQ is checked in Dispatch packet function */
    MAXreg_wr(rHCTL,bmSAMPLEBUS);                                               // update the JSTATUS and KSTATUS bits
    while(!(MAXreg_rd( rHCTL ) & bmSAMPLEBUS ));                                //wait for sample operation to finish
    MAX_busprobe();                                                             //check if anything is connected
    MAXreg_wr( rHIRQ, bmCONDETIRQ );                                            //clear connection detect interrupt                 
    MAXreg_wr( rCPUCTL, 0x01 );                                                 //enable interrupt pin
}

/* MAX3421 state change task and interrupt handler */
void MAX3421E_Task( void )
{
	
    if( (PORTF.IN & PIN6_bm) == 0 ) {
        MaxIntHandler();

    }
    /*
	if( MAX3421E_GPX == 1 ) {
        MaxGpxHandler();
    } 
	*/  
}   

void MaxIntHandler( void )
{
 BYTE HIRQ;
 BYTE HIRQ_sendback = 0x00;
        HIRQ = MAXreg_rd( rHIRQ );                  //determine interrupt source
//        if( HIRQ & bmFRAMEIRQ ) {                   //->1ms SOF interrupt handler
//                    HIRQ_sendback |= bmFRAMEIRQ;
//        }//end FRAMEIRQ handling
        if( HIRQ & bmCONDETIRQ ) {
            MAX_busprobe();
            HIRQ_sendback |= bmCONDETIRQ;
        }
        //if ( HIRQ & bmBUSEVENTIRQ ) {               //bus event is either reset or suspend
        //    usb_task_state++;                       //advance USB task state machine
        //    HIRQ_sendback |= bmBUSEVENTIRQ; 
        //}
        /* End HIRQ interrupts handling, clear serviced IRQs    */
        MAXreg_wr( rHIRQ, HIRQ_sendback );
}
void MaxGpxHandler( void )
{
/* BYTE GPINIRQ;

    GPINIRQ = MAXreg_rd( rGPINIRQ );            //read both IRQ registers
*/
}

void Max_write(char pin, char val) {
    //TODO: Find a better way to do this comparison
    if (pin < 8) {
        // process only if the pin is less than 8

        uint8_t bit = pinToBitMask(pin);
        uint8_t reg = pinToRegister(pin);
        uint8_t out = MAXreg_rd(reg);
        
        if (val == 0) {
            out &= ~bit;
        } else {
            out |= bit;
        }

        MAXreg_wr(reg, out);
    } else {
        //Serial.print(pin);Serial.println(" not a pin");
    }
}

/**
 * Read is performed by calling the regRd function of the USB class
 *
 * @todo Find a better way to check if pin is 0<= pin <= 7
 */
char Max_read(char pin) {

    //TODO: Find a better way to do this comparison
    if (pin < 8) {
        // process only if the pin is less than 8

        uint8_t bit = pinToBitMask(pin);
        uint8_t reg = pinToRegister(pin);
        uint8_t out = MAXreg_rd(reg);
        
        if (out & bit) {
            return 1;
        } else {
            return 0;
        }
    } else {
        //Serial.print(pin);Serial.println(" not a pin");
        return 0;
    }
}