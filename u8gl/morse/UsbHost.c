/*
 * UsbHost.c
 *
 *  Created on: May 13, 2009
 *      Author: Vito Vecchio
 *
 *
 *
 *
 */

#include <stdio.h>
#include <string.h>

#define NAK_LIMIT 200
#define RETRY_LIMIT 3

#define VBUS_ON Host_SPI_Write(rIOPINS2,(Host_SPI_Read(rIOPINS2)|0x08));				// H-GPO7
#define VBUS_OFF Host_SPI_Write(rIOPINS2,(Host_SPI_Read(rIOPINS2)& ~0x08));


static BYTE XfrData[2000];      // Big array to handle max size descriptor data
static BYTE maxPacketSize;		// discovered and filled in by Get_Descriptor-device request in enumerate_device()
static WORD VID,PID,nak_count,IN_nak_count,HS_nak_count;
static unsigned int last_transfer_size;
unsigned volatile long timeval;			// incremented by timer0 ISR
WORD inhibit_send;



void vStartUSBTasks( unsigned portBASE_TYPE uxPriority )
{
	xTaskCreate( vUsbTask, ( signed portCHAR * ) "USB", configMINIMAL_STACK_SIZE, NULL, uxPriority , NULL );
}


BYTE print_err(BYTE err)
{
	if(err)
	{
		printf(">>>>> Error >>>>> ");
		switch(err)
		{
		case 0x01: printf("MAX3421E SIE is busy "); break;
		case 0x02: printf("Bad value in HXFR register "); break;
		case 0x04: printf("Exceeded NAK limit"); break;
		case 0x0C: printf("LS Timeout "); break;
		case 0x0D: printf("FS Timeout "); break;
		case 0x0E: printf("Device did not respond in time "); break;
		case 0x0F: printf("Device babbled (sent too long) "); break;
		default:   printf("Programming error %01X,",err);
		}
	}
	return(err);
}



void vUsbTask(void)
{

	SPIInit();

	printf("---------------------------------------\n\r");
	printf("Maxim USB Driver By Maxim \n\r");
	printf("---------------------------------------\n\r");


	initialize_ARM_Interrupts();



	Host_SPI_Write(rPINCTL,(bmFDUPSPI|bmPOSINT)); // MAX3421E: Full duplex mode, INTLEVEL=0, POSINT=1 for pos edge interrupt pin
	print("done\n\r");
	Reset_Host();	// Jan07_2008: Moved Reset_Host after MAX3421 is put in full duplex mode



	printf("done\n\r");
	Host_SPI_Write(rIOPINS1,0x00);		// seven-segs off
	printf("done\n\r");
	Host_SPI_Write(rIOPINS2,0x00);		// and Vbus OFF (in case something already plugged in)
	printf("done\n\r");


	print("done\n\r");
	aspetta_ms(200);  // aspetta = waits


	VBUS_ON;


	Host_SPI_Write(rPINCTL,(bmFDUPSPI|bmPOSINT));



	while(1)
	{
		detect_device();
		aspetta_ms(200); 			// Some devices require this
		enumerate_device();
		wait_for_disconnect();
	}




}


/*
 *
 *
 * 	       DETECTION DEL DEVICE
 *
 *
 *
 */



void detect_device(void)
{
	int busstate;
	// Activate HOST mode & turn on the 15K pulldown resistors on D+ and D-
	Host_SPI_Write(rMODE,(bmDPPULLDN|bmDMPULLDN|bmHOST)); // Note--initially set up as a FS host (LOWSPEED=0)
	Host_SPI_Write(rHIRQ,bmCONDETIRQ);  // clear the connection detect IRQ
	print("Waiting for device connect\n\r\n\r");

	do 		// See if anything is plugged in. If not, hang until something plugs in
	{
		Host_SPI_Write(rHCTL,bmSAMPLEBUS);			// update the JSTATUS and KSTATUS bits
		busstate = Host_SPI_Read(rHRSL);			// read them
		busstate &= (bmJSTATUS|bmKSTATUS);	// check for either of them high
	}
	while (busstate==0);
	if (busstate==bmJSTATUS)    // since we're set to FS, J-state means D+ high
	{
		Host_SPI_Write(rMODE,(bmDPPULLDN|bmDMPULLDN|bmHOST|bmSOFKAENAB));  // make the MAX3421E a full speed host
		printf("Full-Speed Device Detected\n\r");
		printf("**************************\n\r");
	}
	if (busstate==bmKSTATUS)  // K-state means D- high
	{
		Host_SPI_Write(rMODE,(bmDPPULLDN|bmDMPULLDN|bmHOST|bmLOWSPEED|bmSOFKAENAB));  // make the MAX3421E a low speed host
		printf("Low-Speed Device Detected\n\r");
		printf("*************************\n\r");
	}
}



/*
 *
 *
 *  		ENUMERAZIONE DEVICE
 *
 *
 *
 */






void enumerate_device(void)
{
	static BYTE HR,iCONFIG,iMFG,iPROD,iSERIAL;
	static WORD TotalLen,ix;
	static BYTE len,type,adr,pktsize;
	// SETUP bytes for the requests we'll send to the device
	static BYTE Set_Address_to_7[8]      		= {0x00,0x05,0x07,0x00,0x00,0x00,0x00,0x00};
	static BYTE Get_Descriptor_Device[8]  		= {0x80,0x06,0x00,0x01,0x00,0x00,0x00,0x00}; // code fills in length field
	static BYTE Get_Descriptor_Config[8]  		= {0x80,0x06,0x00,0x02,0x00,0x00,0x00,0x00};
	static BYTE str[8] = {0x80,0x06,0x00,0x03,0x00,0x00,0x40,0x00};	// Get_Descriptor-String template. Code fills in idx at str[2].

	// Issue a USB bus reset
	printf("Issuing USB bus reset\n\r");
	Host_SPI_Write(rHCTL,bmBUSRST);           // initiate the 50 msec bus reset
	while(Host_SPI_Read(rHCTL) & bmBUSRST);  // Wait for the bus reset to complete

	// Wait some frames before programming any transfers. This allows the device to recover from
	// the bus reset.
	aspetta_ms(200);

	// Get the device descriptor.
	maxPacketSize = 8;				// only safe value until we find out
	Host_SPI_Write(rPERADDR,0);   			// First request goes to address 0
	Get_Descriptor_Device[6]=8;		// wLengthL
	Get_Descriptor_Device[7]=0;		// wLengthH
	printf("First 8 bytes of Device Descriptor ");
	HR = CTL_Read(Get_Descriptor_Device);  	// Get device descriptor into XfrData[]

	if(print_err(HR)) return;	// print_error() does nothing if HRSL=0, returns the 4-bit HRSL.

	printf("(%u/%u NAKS)\n\r",IN_nak_count,HS_nak_count);		// Show NAK count for data stage/status stage
	maxPacketSize = XfrData[7];
	for (ix=0; ix<last_transfer_size;ix++)
		printf("%02X ",(BYTE*)XfrData[ix]);
	printf("\n\r");
	printf("EP0 maxPacketSize is %02u bytes\n\r",maxPacketSize);

	// Issue another USB bus reset
	printf("Issuing USB bus reset\n\r");
	Host_SPI_Write(rHCTL,bmBUSRST);           // initiate the 50 msec bus reset
	while(Host_SPI_Read(rHCTL) & bmBUSRST);  // Wait for the bus reset to complete
	aspetta_ms(200);

	// Set_Address to 7 (Note: this request goes to address 0, already set in PERADDR register).
	printf("Setting address to 0x07\n\r");
	HR = CTL_Write_ND(Set_Address_to_7);   // CTL-Write, no data stage
	if(print_err(HR)) return;

	aspetta_ms(30);           // Device gets 2 msec recovery time
	Host_SPI_Write(rPERADDR,7);       // now all transfers go to addr 7

	// Get the device descriptor at the assigned address.
	Get_Descriptor_Device[6]=0x12;			// fill in the real device descriptor length
	printf("\n\rDevice Descriptor ");
	HR = CTL_Read(Get_Descriptor_Device);  // Get device descriptor into XfrData[]
	if(print_err(HR)) return;
	printf("(%u/%u NAKS)\n\r",IN_nak_count,HS_nak_count);
	printf  ("-----------------\n\r");

	VID 	= XfrData[8] + 256*XfrData[9];
	PID 	= XfrData[10]+ 256*XfrData[11];
	iMFG 	= XfrData[14];
	iPROD 	= XfrData[15];
	iSERIAL	= XfrData[16];

	for (ix=0; ix<last_transfer_size;ix++)
		printf("%02X ",(BYTE*)XfrData[ix]);
	printf("\n\r");
	printf("This device has %u configuration\n\r",XfrData[17]);
	printf("Vendor  ID is 0x%04X\n\r",VID);
	printf("Product ID is 0x%04X\n\r",PID);
	//
	str[2]=0;	// index 0 is language ID string
	str[4]=0;	// lang ID is 0
	str[5]=0;
	str[6]=4;	// wLengthL
	str[7]=0;	// wLengthH

	HR = CTL_Read(str);  	// Get lang ID string
	if (!HR)				// Check for ACK (could be a STALL if the device has no strings)
	{
		printf("\n\rLanguage ID String Descriptor is ");
		for (ix=0; ix<last_transfer_size;ix++)
			printf("%02X ",(BYTE*)XfrData[ix]);
		str[4]=XfrData[2]; 	// LangID-L
		str[5]=XfrData[3];	// LangID-H
		str[6]=255; 		// now request a really big string
	}
	if(iMFG)
	{
		str[2]=iMFG; 			// fill in the string index from the device descriptor
		HR = CTL_Read(str);  	// Get Manufacturer ID string
		printf("\n\rManuf. string is  \"");
		for (ix=2; ix<last_transfer_size;ix+=2)
			printf("%c",(BYTE*)XfrData[ix]);
		printf("\"\n\r");
	}
	else printf("There is no Manuf. string\n\r");

	if(iPROD)
	{
		str[2]=iPROD;
		HR = CTL_Read(str);  // Get Product ID string
		printf("Product string is \"");
		for (ix=2; ix<last_transfer_size;ix+=2)
			printf("%c",(BYTE*)XfrData[ix]);
		printf("\"\n\r");
	}
	else printf("There is no Product string\n\r");

	if(iSERIAL)
	{
		str[2]=iSERIAL;
		HR = CTL_Read(str);  // Get Serial Number ID string
		printf("S/N string is     \"");
		for (ix=2; ix<last_transfer_size;ix+=2)
			printf("%c",(BYTE*)XfrData[ix]);
		printf("\"\n\r");
	}
	else printf("There is no Serial Number");

	// Get the 9-byte configuration descriptor

	printf("\n\r\n\rConfiguration Descriptor ");
	Get_Descriptor_Config[6]=9;	// fill in the wLengthL field
	Get_Descriptor_Config[7]=0;	// fill in the wLengthH	field

	HR = CTL_Read(Get_Descriptor_Config);  // Get config descriptor into XfrData[]
	if(print_err(HR)) return;
	printf("(%u/%u NAKS)\n\r",IN_nak_count,HS_nak_count);
	printf  ("------------------------\n\r");

	for (ix=0; ix<last_transfer_size;ix++)
		printf("%02X ",(BYTE*)XfrData[ix]);

	// Now that the full length of all descriptors (Config, Interface, Endpoint, maybe Class)
	// is known we can fill in the correct length and ask for the full boat.

	Get_Descriptor_Config[6]=XfrData[2];	// LengthL
	Get_Descriptor_Config[7]=XfrData[3];	// LengthH
	HR = CTL_Read(Get_Descriptor_Config);  // Get config descriptor into XfrData[]

	printf("\n\rFull Configuration Data");
	for (ix=0; ix<last_transfer_size;ix++)
	{
		if((ix&0x0F)==0) printf("\n\r");		// CR every 16 numbers
		printf("%02X ",(BYTE*)XfrData[ix]);
	}
	iCONFIG = XfrData[6];	// optional configuration string

	printf("\n\rConfiguration %01X has %01X interface",XfrData[5],XfrData[4]);
	if (XfrData[4]>1) printf("s");
	printf("\n\rThis device is ");
	if(XfrData[7] & 0x40)	printf("self-powered\n\r");
	else					printf("bus powered and uses %03u milliamps\n\r",XfrData[8]*2);
	//
	// Parse the config+ data for interfaces and endpoints. Skip over everything but
	// interface and endpoint descriptors.
	//
	TotalLen=last_transfer_size;
	ix=0;
	do
	{
		len=XfrData[ix];		// length of first descriptor (the CONFIG descriptor)
		type=XfrData[ix+1];
		adr=XfrData[ix+2];
		pktsize=XfrData[ix+4];

		if(type==0x04)			// Interface descriptor?
			printf("Interface %u, Alternate Setting %u has:\n\r",XfrData[ix+2],XfrData[ix+3]);
		else if(type==0x05)		// check for endpoint descriptor type
		{
			printf("--Endpoint %u",(adr&0x0F));
			if (XfrData[ix+2]&0x80) printf("-IN  ");
			else printf("-OUT ");
			printf("(%02u) is type ",(BYTE)pktsize);

			switch(XfrData[ix+3]&0x03)
			{
			case 0x00:
				printf("CONTROL\n\r"); break;
			case 0x01:
				printf("ISOCHRONOUS\n\r"); break;
			case 0x02:
				printf("BULK\n\r"); break;
			case 0x03:
				printf("INTERRUPT with a polling interval of %u msec.\n\r",XfrData[ix+6]);
			}
		}
		ix += len;				// point to next descriptor
	}
	while (ix<TotalLen);
	//
	if(iCONFIG)
	{
		str[2]=iCONFIG;
		HR = CTL_Read(str);  // Get Config string
		printf("\n\rConfig string is \"");
		for (ix=2; ix<last_transfer_size;ix+=2)
			printf("%c",(BYTE*)XfrData[ix]);
		printf("\"\n\r");
	}
	else printf("There is no Config string\n\r");

}





/*
 *
 *
 *       SENDPACKET
 *
 *
 *
 */


BYTE Send_Packet(BYTE token,BYTE endpoint)
{
	BYTE resultcode,retry_count;
	retry_count = 0;
	nak_count = 0;
	//
	while(1) 	// If the response is NAK or timeout, keep sending until either NAK_LIMIT or RETRY_LIMIT is reached.
		// Returns the HRSL code.
	{
		Host_SPI_Write(rHXFR,(token|endpoint));         // launch the transfer
		while(!(Host_SPI_Read(rHIRQ) & bmHXFRDNIRQ));  // wait for the completion IRQ
		Host_SPI_Write(rHIRQ,bmHXFRDNIRQ);              // clear the IRQ
		resultcode = (Host_SPI_Read(rHRSL) & 0x0F);    // get the result
		if (resultcode==hrNAK)

		{
			nak_count++;
			if(nak_count==NAK_LIMIT) break;
			else continue;
		}

		if (resultcode==hrTIMEOUT)
		{
			retry_count++;
			if (retry_count==RETRY_LIMIT) break;    // hit the max allowed retries. Exit and return result code
			else continue;
		}
		else break;                           	// all other cases, just return the success or error code
	}
	return(resultcode);
}



/*
 *
 *      RESETTA
 *
 *
 */



void Reset_Host(void)
{
	print("RESET\n\r");
	//Host_wr_HostReg(rUSBCTL,0x20/*bmCHIPRES*/);  // chip reset This stops the oscillator
	//print("RESETTATO\n\r");

	Host_SPI_Write(rUSBCTL,0x00); // remove the reset
	print("prima");
	while(!(Host_SPI_Read(rUSBIRQ) & bmOSCOKIRQ));
	// hang until the PLL stabilizes

	print("DOPO\n\r");
}
/*
 *
 *
 *
 *         CONTROL_TRANSFER_READ
 *
 *
 *
 */



// ----------------------------------------------------
// CONTROL-Read Transfer. Get the length from SUD[7:6].
// ----------------------------------------------------
BYTE CTL_Read(BYTE *pSUD)
{
	BYTE  resultcode;
	WORD	bytes_to_read;
	bytes_to_read = pSUD[6] + 256*pSUD[7];

	// SETUP packet
	Host_Write_N_Bytes(rSUDFIFO,8,pSUD);      		// Load the Setup data FIFO
	resultcode=Send_Packet(tokSETUP,0);   	// SETUP packet to EP0
	if (resultcode) return (resultcode); 		// should be 0, indicating ACK. Else return error code.
	// One or more IN packets (may be a multi-packet transfer)
	Host_SPI_Write(rHCTL,bmRCVTOG1);            		// FIRST Data packet in a CTL transfer uses DATA1 toggle.
	//  last_transfer_size = IN_Transfer(0,bytes_to_read);     // In transfer to EP-0 (IN_Transfer function handles multiple packets)
	resultcode = IN_Transfer(0,bytes_to_read);
	if(resultcode) return (resultcode);

	IN_nak_count=nak_count;
	// The OUT status stage
	resultcode=Send_Packet(tokOUTHS,0);
	if (resultcode) return (resultcode);   // should be 0, indicating ACK. Else return error code.
	return(0);    // success!
}

/*
 *
 *
 * 			SCRITTURA_A_ENDPOINT
 *
 *
 */



// -----------------------------------------------------------------------------------
// IN Transfer to arbitrary endpoint. Handles multiple packets if necessary. Transfers
// "length" bytes.
// -----------------------------------------------------------------------------------
// Do an IN transfer to 'endpoint'. Keep sending INS and saving concatenated packet data
// in array Xfr_Data[] until 'numbytes' bytes are received. If no errors, returns total
// number of bytes read. If any errors, returns a byte with the MSB set and the 7 LSB
// indicating the error code from the "launch transfer" function.
//
BYTE IN_Transfer(BYTE endpoint,WORD INbytes)
{
	BYTE resultcode,j;
	BYTE pktsize;
	unsigned int xfrlen,xfrsize;

	xfrsize = INbytes;
	xfrlen = 0;

	while(1) // use a 'return' to exit this loop.
	{
		resultcode=Send_Packet(tokIN,endpoint);     	// IN packet to EP-'endpoint'. Function takes care of NAKS.
		if (resultcode) return (resultcode);  		// should be 0, indicating ACK. Else return error code.
		pktsize=Host_SPI_Read(rRCVBC);                        // number of received bytes
		for(j=0; j<pktsize; j++)                      // add this packet's data to XfrData array
			XfrData[j+xfrlen] = Host_SPI_Read(rRCVFIFO);
		Host_SPI_Write(rHIRQ,bmRCVDAVIRQ);                     // Clear the IRQ & free the buffer
		xfrlen += pktsize;                            // add this packet's byte count to total transfer length
		//
		// The transfer is complete under two conditions:
		// 1. The device sent a short packet (L.T. maxPacketSize)
		// 2. 'INbytes' have been transferred.
		//
		if ((pktsize < maxPacketSize) || (xfrlen >= xfrsize))    // have we transferred 'length' bytes?
			// 	 return xfrlen;
		{
			last_transfer_size = xfrlen;
			return(resultcode);
		}
	}
}

/*
 *
 *
 *       CONTROL_TRANSFER_WRITE
 *
 *
 *
 *
 */


// -----------------------------------------------------------------------------------
// Control-Write with no data stage. Assumes PERADDR is set and the SUDFIFO contains
// the 8 setup bytes. Returns with result code = HRSLT[3:0] (HRSL register).
// If there is an error, the 4 MSB's of the returned value indicate the stage 1 or 2.
// -----------------------------------------------------------------------------------
BYTE CTL_Write_ND(BYTE *pSUD)
{
	BYTE resultcode;
	Host_Write_N_Bytes(rSUDFIFO,8,pSUD);
	// 1. Send the SETUP token and 8 setup bytes. Device should immediately ACK.
	resultcode=Send_Packet(tokSETUP,0);    // SETUP packet to EP0
	if (resultcode) return (resultcode);   // should be 0, indicating ACK.

	// 2. No data stage, so the last operation is to send an IN token to the peripheral
	// as the STATUS (handshake) stage of this control transfer. We should get NAK or the
	// DATA1 PID. When we get the DATA1 PID the 3421 automatically sends the closing ACK.
	resultcode=Send_Packet(tokINHS,0);   // This function takes care of NAK retries.
	if(resultcode) return (resultcode);  // should be 0, indicating ACK.
	else  return(0);
}






/*
 *
 *
 *        INIT_INTERRUPTS
 *
 *
 *
 *
 */



// Timer Counter 0 Interrupt executes each 20ms @ 48 MHz CPU Clock
// Increment counters timeval for general program use.
//
void tc0 (void)
{
	//static unsigned char blinkcount;
	++timeval;				// background program can reset this
	T0IR        = 1;    	// Clear interrupt flag
	VICVectAddr = 0;		// Dummy write to indicate end of interrupt service

}

// Timer Counter 1 Interrupt executes each 50ms @ 48 MHz CPU Clock
// Checks the start/stop button, clears inhibit_send if pressed.
// Also increments blinkcount and blinks the D4 led when at limit.
//
void tc1 (void)
{
	static unsigned char dum,blinkcount;


	T1IR        = 1;    			// Clear interrupt flag
	VICVectAddr = 0;				// Dummy write to indicate end of interrupt service

}

void initialize_ARM_Interrupts(void)
{
	// Set up the Timer Counter 0 Interrupt
	// Used to blink the activity light


	print("inizializzazione ARM interrupts.....");
	T0MR0 = 960000;						 // Match Register 0: 20 msec(50 Hz) with 48 MHz clock
	T0MCR = 3;                           // Match Control Reg: Interrupt(b0) and Reset(b1) on MR0
	T0TCR = 1;                           // Timer0 Enable
	VICVectAddr1 = (unsigned long)tc0;   // Use slot 1, second highest vectored IRQ priority.
	VICVectCntl1 = 0x20 | 4;             // 0x20 is the interrupt enable bit, 0x04 is the TIMER0 channel number
	VICIntEnable = 0x00000010;           // Enable Timer0 Interrupt bit 4 (1 sets the bit)

	// Set up the Timer Counter 1 Interrupt
	// Used to check the send/stop button PB5

	T1MR0 = 4800000;					 // Match Register 0: 100 msec (10Hz) with 48 MHz clock
	T1MCR = 3;                           // Match Control Reg: Interrupt(b0) and Reset(b1) on MR0
	T1TCR = 1;                           // Timer1 Enable
	VICVectAddr3 = (unsigned long)tc1;   // Use slot 3, lowest vectored IRQ priority in this app
	VICVectCntl3 = 0x20 | 5;             // 0x20 is the interrupt enable bit, 0x05 is the TIMER5 channel number
	VICIntEnable = 0x00000020;           // Enable Timer1 Interrupt bit 5 (1 sets the bit)

	// Set up the EINT0 (P0.16) interrupt (MAX3420E Interrupt pin)  --> ERA PER IL MAX 3420 e non mi interessa

	// Set up the EINT2 (P0.15) interrupt (MAX3421E Interrupt pin)--not used, just an example.
	// Set for pos-edge  --- ho cambiato con quello in posizione sul GPIO_IO_P7



	Host_SPI_Write(rPINCTL,(bmFDUPSPI|bmPOSINT)); // MAX3421E: INTLEVEL=0, POSINT=1 for pos edge interrupt pin
	//EXTMODE 	|= 0x04;	// EINT2 is edge-sensitive
	//EXTPOLAR	|= 0x04; 	// positive edge
	//EXTINT		= 4;		// clear the IRQ which may have been set by the above two instructions (writing 1 clears it)
	//
	//VICVectAddr2 = (unsigned long)INT3421; 	// Use slot 2
	//VICVectCntl2 = 0x20 | 16;          		// 0x20 is the interrupt enable bit, 16(D) is the EINT2 channel number
	//VICIntEnable = 0x00010000;          	// Enable EINT2 interrupt bit 16 (1 sets the bit)

	eint2Init();



	print("Finita\n\r");

}

static void eint2ISR_Handler (void)
{
	SCB_EXTINT |= SCB_EXTINT_EINT2;

	IO0SET = GPIO_IO_P7;

	VIC_VectAddr = (unsigned portLONG) 0;
}

void eint2ISR (void) __attribute__ ((naked));
void eint2ISR (void)
{
	//portSAVE_CONTEXT ();
	eint2ISR_Handler ();
	// portRESTORE_CONTEXT ();
}


void eint2Init(void);
void eint2Init (void)
{
	portENTER_CRITICAL ();

	PCB_PINSEL0 = (PCB_PINSEL0 & ~PCB_PINSEL0_P07_MASK) | PCB_PINSEL0_P07_EINT2;

	SCB_EXTPOLAR &= ~SCB_EXTPOLAR_EINT2;
	SCB_EXTMODE  |=  SCB_EXTMODE_EINT2;
	SCB_EXTINT   |=  SCB_EXTINT_EINT2;

	VIC_IntSelect &= ~VIC_IntSelect_EINT2;
	VIC_VectAddr5 = (portLONG) eint2ISR;
	VIC_VectCntl5 = VIC_VectCntl_ENABLE | VIC_Channel_EINT2;
	VIC_IntEnable = VIC_IntEnable_EINT2;

	portEXIT_CRITICAL ();
}






/*
 *
 *      INTERRUPT HANDLER PER USB
 *
 */

void INT3421 (void)
{
	EXTINT      = 4;    // Clear EINT2 interrupt flag (b0)

	GPIO0_FIOCLR = GPIO_IO_P7;

	VIC_VectAddr = 0;	// Dummy write to indicate end of interrupt service
}



/*
 *
 *
 *        GESTIONE_ERRORI
 *
 *
 *
 */






/*
 *
 *       ASPETTA_DISCONNECT
 *
 *
 */





void wait_for_disconnect(void)
{

	printf("\n\rWaiting for device disconnect\n\r");

	Host_SPI_Write(rHIRQ,bmCONDETIRQ);    	// clear the disconect IRQ

	while(!(Host_SPI_Read(rHIRQ) & bmCONDETIRQ)) ; 		// hang until this changes

	Host_SPI_Write(rMODE,bmDPPULLDN|bmDMPULLDN|bmHOST);	// turn off frame markers

	printf("\n\rDevice disconnected\n\r\n\r");

}








