/*
 * rotor_controller.c
 *
 * Created: 2015-05-19 16:01:38
 *  Author: pelu0072
 */ 


#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include "can/can.h"
#include "can/MCP2515.h"
#include "sdcard_driver.h"
#include "menus_def.h"
#include "Controller_def.h"
#include "can/can.h"
#include "sdcard_driver.h"
#include "sdcard/sd_raw.h"
#include "u8g/u8g.h"
#include "CAN_Queue.h"
#include "yeasu/FT-857D.h"
#include "io_driver.h"
#include "Sequencer.h"
#include "usb/USBhost.h"
#include "scheduler.h"



u8g_t u8g;
void draw();
void setUpSerial_rpt();
void setUpSerial_main();
void setClockTo32MHz();
static int uart_putchar (char c, FILE *stream);
int uart_getchar(FILE *stream);
FILE usart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
extern char key_pressed;
radio rs232radio;
Amp	amplifier;


int rotor_curent =0;
int rotor_target =0;
int curent_menu =0;

int buttion_one();
int buttion_two();

void screen_int();
void draw_angel_circle(int angel, int type );
void send_data_to_freq();
void send_data_to_pi();
void trasmit_slide();
void easy_com_angel();
void get_band();
void main_screen();
int Select_buttion();
void send_data_on_canbus();
void send_stoptx_message(int unit);
void send_tx_message(int unit);

// New can message
int new_can_message =0;


void setup()
{

	//toogle_alert();
	setClockTo32MHz();
	// set direcotion of lcd
	PORTK.DIRSET =0xFF;
	PORTJ.DIRSET =0xFF;
	PORTE.DIRSET =0xB0;
	// dir of inputs
	PORTH.DIRCLR    =    PIN2_bm;
    PORTH.PIN2CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN3_bm;
    PORTH.PIN3CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN4_bm;
	PORTH.PIN4CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN5_bm;
	PORTH.PIN5CTRL  =    PORT_OPC_PULLUP_gc;
	PORTH.DIRCLR    =    PIN4_bm;
	setup_buttons(); //*burrons config *//
	
	// port D setup interupt
	PORTD.DIRCLR    =    PIN1_bm;
	
	// interupt rotary encoder
	PORTH.DIR = 0x00; // Port D als Eingang
    PORTH.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_RISING_gc;// PD0: Pull up, erkenne Rising Edge
	PORTH.PIN1CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_RISING_gc;// PD0: Pull up, erkenne Rising Edge
	PORTH.PIN7CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_RISING_gc;// PD0: Pull up, erkenne Rising Edge

	PORTH.INT0MASK = PIN0_bm; // PD0 lÃ¶st Interrupt 0 aus. 
	PORTH.INT1MASK = PIN1_bm;
  
	PORTH_INTCTRL = PORT_INT0LVL_MED_gc | PORT_INT1LVL_MED_gc; // Port D Interrupt 0: Medium Level
	
	
	//PORTH PTT INT
	PORTA.INT0MASK = PIN0_bm; // PD0 lÃ¶st Interrupt 0 aus. 
	PORTA_INTCTRL = PORT_INT0LVL_MED_gc; // Port D Interrupt 0: Medium Level
	PORTA.DIRSET = PIN2_bm;
	
	
	// Can pin interupt
	
	PORTD.OUTCLR = PIN1_bm;
	PORTD.PIN1CTRL = PORT_ISC_FALLING_gc | PORT_OPC_PULLUP_gc;// PD0: Pull up, erkenne Rising Edge
	PORTD.INT0MASK = PIN1_bm; // PD0 lÃ¶st Interrupt 0 aus. 
    PORTD_INTCTRL = PORT_INT0LVL_MED_gc; // Port D Interrupt 0: Medium Level
	PORTD.INTFLAGS = 0x00;
  
  
  //Enable Medium-Level-Interrupts 
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
  //Enable Interrupts 
	sei();


	// LCD REGISTER TOLKEN
	int DR = 2;
	int CR =3;
	// display deklaration

	u8g_Init8Bit( &u8g,&u8g_dev_ks0108_128x64 , u8g_Pin(DR,0), u8g_Pin(DR,1), u8g_Pin(DR,2), u8g_Pin(DR,3), PN(DR,4), PN(DR,5), PN(DR,6), PN(CR,6),PN(CR,5),PN(CR,0),PN(CR,1),PN(CR,4),PN(CR,3),PN(CR,2));


	//sei();
	stdout = stdin = &usart_str;
		// canbus init
	CAN_init();
	//clear_alert();
	

	
	setup_timmer();	 
	//uarts
	setUpSerial_rpt();
	setUpSerial_main();

}


// set pll to 32mhz
void setClockTo32MHz()
{
	
	// 	CCP = CCP_IOREG_gc;              // disable register security for oscillator update
	// 	OSC.CTRL = OSC_RC32MEN_bm;       // enable 32MHz oscillator
	// 	while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator to be ready
	// 	CCP = CCP_IOREG_gc;              // disable register security for clock update
	// 	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; // switch to 32MHz clock
	OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc ;
	OSC.CTRL |= OSC_XOSCEN_bm ; // enable it
	while( (OSC.STATUS & OSC_XOSCRDY_bm) == 0 ){} // wait until it's stable

	// The external crystal is now running and stable.
	// (Note that it's not yet selected as the clock source)
	// Now configure the PLL to be eXternal oscillator * 2
	OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | 2 ;

	// now enable the PLL...
	OSC.CTRL |= OSC_PLLEN_bm ; // enable the PLL...
	while( (OSC.STATUS & OSC_PLLRDY_bm) == 0 ){} // wait until it's stable

	// And now, *finally*, we can switch from the internal 2Mhz clock to the PLL
	CCP = CCP_IOREG_gc;	// protected write follows
	CLK.CTRL = CLK_SCLKSEL_PLL_gc;	// The System clock is now  PLL (16Mhz * 2)
	// ==============================================


}
// global for all tasks 

int ptt_test =0;
void Main_task(void);
void read_usb_Task(void);
void Send_can_Task(void);
void Send_can_heartbeat(void);
void Recive_data_from_can(void);


void Setup_main()
{
		printf("Bootloader Done!\n");
		toogle_alert();
		printf("INT devices!\n");
		setup();
		printf("Done!\n");

		// test
		printf("Loading SD card!\n");
		sd_raw_init();
		sd_card_open();
		screen_int();
		printf("Done!\n");

		_delay_ms(300);
		// USB HOST
		
		// sd_file_new(filename);
		// sd_file_open(filename);
		// sd_file_write(filetext);
		// sd_file_close();

		// struct form main controller functions
		//struct cont *controller;
		printf("Allocate memory menus!\n");
		sei();

		_delay_ms(500);
		
		printf("System Online!\n");
		clear_alert();
		

		setUpSerial_rpt();
		main_setup();

}
/*** tasks */


/*read usb host */
void read_system_Task()
{
	
	
	main_usb();
	// readcan
	Send_can_Task();
	if (key_pressed == '1')
	{
		beep(40);
	}
			
}

// alocated in main for memory
can_message_t * Rxmsg;


/*** Send data on canbus task */
void Send_can_Task(void)
{
	
//	printf("Send can \n");
	if (can_queue_is_empty() ==0)
	{
		
		while(can_queue_is_empty() ==0)
		{
			
			send_data_on_canbus();
		}
	}
	if (can_queue_is_empty() ==1)
	{
		CAN_data_receive(Rxmsg);
	}
	
}
int heartbeat= 0;
void print_heartbeat()
{
	heartbeat++;
	printf("2;h;%i\n",heartbeat);
	printf("1;%s;%i;%ld;%ld;\n",rs232radio.model,get_amp_id(),rs232radio.freqvensy,rs232radio.band);
}




/*** Send data on canbus heartbeat */
void Send_can_heartbeat(void)
{
	send_data_to_pi();
	send_data_to_freq();
}
/*** Main taks  */

void Main_task(void)
{

	//can_message_t  Rxmsg;

		
	/*if (Rxmsg.data[0] !=128)
	{
		//printf("fwd %i \n",Rxmsg.data[0]);
	}
	*/


	if ( rs232radio.ptt == 1)
	{
		if (ptt_test ==0)
		{
			//set_amp_id(2);
			TX_sequens();
			ptt_test =1;
		}
			
			
		if (Rxmsg->data[1] == 7)
		{
			amplifier.power_fwd =((Rxmsg->data[2] << 8) | Rxmsg->data[3]);
			amplifier.power_rev =((Rxmsg->data[4] << 8) | Rxmsg->data[5]);
				
				
		}
			
		amplifier.power_max = 250;
		trasmit_slide();
	}
	else
	{
		if (ptt_test == 1)
		{
			RX_sequens();
			ptt_test =0;
				
		}
		main_screen();
	}
	
	
	
	//heartbeat++;
	//printf("Heartbeat %i\n",heartbeat);
}


int y_pos = 0;  // global variable
extern void radio_pull_data_thread();

int main(void)
{	
	rs232radio.enable = 0;
	rs232radio.rs232_prescale=0;
	rs232radio.radio_rs232 = 206;
	rs232radio.model="Yesu ft-8xx";
	//debug
	rs232radio.amp_id=2;

	Setup_main();
	scedular_setup();
	beep(60);
	
	
	
	addTask(2, read_system_Task, 2);
	addTask(3, Main_task, 2);
	addTask(4, radio_pull_data_thread, 15);
	addTask(5, Send_can_heartbeat, 4);
	addTask(1, print_heartbeat, 10);
	addTask(6, get_band, 4);


	
	Rxmsg =malloc(sizeof(can_message_t *));
	
	for(;;)	
	{	
		dispatchTasks();	
	}
	free(Rxmsg); 
	return 0; /*** Will newer get here :) */
		
			
}



/** Send a mode pacet for pi i band*/ 
void send_data_to_pi()
{
	can_message_t  pimsg;
	pimsg.msg_id=200;
	pimsg.data_length=7;
	pimsg.data[0] = 252;
	/* send band in 16 bit over can*/
	pimsg.data[1] = (rs232radio.band& 0xFF);
	pimsg.data[2] = (rs232radio.band >>8);
	pimsg.data[3] = rs232radio.meter;
	pimsg.data[4] = rs232radio.ptt;
	pimsg.data[5] = rs232radio.amp_id;
	pimsg.data[6] = rs232radio.mode_id;

	
	can_queue_Enqueue(pimsg);
	
	
}
/** create a 32bit int from freq to send to canbus for freq*/ 
void send_data_to_freq()
{	
		/** create a 32bit int to send to pi for freq*/ 
		uint32_t Freq_to_send;
		Freq_to_send= rs232radio.freqvensy;
		can_message_t  pimsg;
		pimsg.msg_id=200;
		pimsg.data_length=5;
		pimsg.data[0]= 253;
		pimsg.data[1]= Freq_to_send;
		pimsg.data[2]= Freq_to_send >> 8;
		pimsg.data[3]= Freq_to_send >> 16;
		pimsg.data[4]= Freq_to_send >> 24;
		can_queue_Enqueue(pimsg);
	

}
/**
Sending the CAN Buss messasge to MCP2515 with a delay
*/
void send_data_on_canbus()
{

	//printf("send_data\n");
	can_message_t send;
	send = can_queue_Front();
	//printf("%i\n",send.data[0]);
	CAN_message_send(&send);
	can_queue_Dequeue();
	_delay_ms(5);
	

	
	

}

void trasmit_slide()
{
		u8g_FirstPage(&u8g);
		//float procennt_fwd =200/amplifier.power_max;
		double procennt_fwd =(100 * amplifier.power_fwd) / amplifier.power_max;
		double procennt_rev =(100 * amplifier.power_rev) / amplifier.power_max;
		int fwd = 2+(procennt_fwd)*1.2;
		int ref = 2+(procennt_rev*1.2);
		//float swr = (amplifier.power_rev /amplifier.power_fwd)*100;
		float swr=0;
		char text[20];
	
		do
		{
			u8g_SetFont(&u8g, u8g_font_6x10);
			u8g_DrawRFrame(&u8g, 1, 1, 126, 15, 2);
			u8g_DrawStr(&u8g, 1, 17, "FWD Power:");
		
			sprintf(text,"%i W",amplifier.power_fwd);
			u8g_DrawStr(&u8g, 67, 17, text);
			
			u8g_DrawRFrame(&u8g, 1, 28, 126, 15, 2);
			
			sprintf(text,"%i W",amplifier.power_rev);
			u8g_DrawStr(&u8g, 1, 45, "REF Power:");
			u8g_DrawStr(&u8g, 67,45 ,text);

			// bars fwd
			u8g_DrawBox(&u8g,3,3,fwd,11);
			//ref
			u8g_DrawBox(&u8g,3,30,ref,11);
			
			sprintf(text,"SWR : 1:%.03f ",swr);
			u8g_DrawStr(&u8g,1,54 ,text);
			
			if(rs232radio.meter == 1)
			{
				sprintf(text, "%ldM", rs232radio.band);
				u8g_DrawStr(&u8g, 110, 54,text );

			}else
			{
				sprintf(text, "%ldcM", rs232radio.band);
				u8g_DrawStr(&u8g, 110, 54,text );
			}
			
					
		} while ( u8g_NextPage(&u8g) );
	
		
}


void screen_int()
{

	u8g_FirstPage(&u8g);
    do
    {
	  u8g_SetFont(&u8g, u8g_font_6x10);
	  u8g_DrawRFrame(&u8g, 1, 1, 126, 62, 2);
      u8g_DrawStr(&u8g, 5, 10, "ANTENNA CONTROLLER");
	  u8g_DrawStr(&u8g, 5, 20, "BY SA2BLV");
	  
	  u8g_DrawStr(&u8g, 5, 50, "Loading ....");
 
    } while ( u8g_NextPage(&u8g) );
// 	u8g_Delay(100);
 	
}

int max_antennas =7;
int meny_selected =0;
int hide_meny =0;

void main_screen()
{
	//meny_selectors(menu);
	u8g_FirstPage(&u8g);
	
	char menu_test[7][20] ={"Rotor","Radio","BUSS","CAN","test4"} ;
		
	char str[8];
	do
	{
	  
		uint8_t  h;
		if (meny_selected <0)
		{
			meny_selected=0;
		}
		if (meny_selected >4)
		{
			meny_selected =4;
		}
		
		
		if (hide_meny == 0)
		{
		
			  u8g_SetFont(&u8g, u8g_font_6x10);
			  u8g_DrawFrame(&u8g, 70, 0, 58, 64);
			  u8g_SetFontRefHeightText(&u8g);
			  u8g_SetFontPosTop(&u8g);
			  //get_band();
			  //button_test();
	  
			  h = u8g_GetFontAscent(&u8g)-u8g_GetFontDescent(&u8g);
	  

	  
			  for(int i=0;i <= 4; i++)
			  {
				  u8g_SetDefaultForegroundColor(&u8g);
				// bakgrund_meny
				 if(meny_selected == i)
				 {
					  u8g_DrawBox(&u8g, 71, i*h+1, 57, 11);     // draw cursor bar
					  u8g_SetDefaultBackgroundColor(&u8g);	  
				 }
				  // skriver ut Antenner
		
				 u8g_DrawStr(&u8g, 72, 10*i, menu_test[i]);
	 
		  
				  if (meny_selected == 4)
					u8g_SetDefaultForegroundColor(&u8g);
		

			  }	  
		  }
		  
		  /** button segment */
		  if (buttion_one() == 1 || key_pressed == 'b')
		  {
			  if (hide_meny == 0)
			  {
				  hide_meny = 1;
				  // add a press delay
				  while(buttion_one()  ==1);
				  
			  }
			  else
			  {
				  // add a press delay
				  while(buttion_one()  ==1);
				  hide_meny =0;
			  }
		  }
	  

	if(meny_selected == 0)
	{
			draw_angel_circle(rotor_curent,0);
			//draw_angel_circle(rotor_target,1);
		
		// showing the band
		
	
	

		if(rs232radio.meter == 1)
		{
	
			sprintf(str, "%ldM", rs232radio.band);
			u8g_DrawStr(&u8g, 2, 52,str );

		}else
		{
			sprintf(str, "%ldcM",rs232radio.band);
			u8g_DrawStr(&u8g, 2, 52,str );
		}
		
		//Show_Radio_freq(2,10);
		
	

		
	
	}
	else if (meny_selected == 1)
	{
		u8g_SetFont(&u8g, u8g_font_6x10);
	

		char freq_basform[10];
			sprintf(freq_basform, "%ld", rs232radio.freqvensy);
	
			if (rs232radio.band >30 && rs232radio.meter == 1)
			{
				sprintf(str, "%.1s.%.3s.%.2s", freq_basform,freq_basform+1,freq_basform+4);
			}
			else if (rs232radio.band >3 && rs232radio.meter == 1)
			{
				sprintf(str, "%.2s.%.3s.%.2s", freq_basform,freq_basform+2,freq_basform+5);
			}
			else
			{
				sprintf(str, "%.3s.%.3s.%.2s",freq_basform,freq_basform+3,freq_basform+6);
			
			}
		
			u8g_DrawStr(&u8g, 2, 2,"Frequency:" );
			u8g_DrawStr(&u8g, 2, 12,str );
	
		u8g_DrawStr(&u8g, 2, 22,"Band:" );
		if(rs232radio.meter == 1)
		{
			sprintf(str, "%ldM", rs232radio.band);
			u8g_DrawStr(&u8g, 40, 22,str );

		}else
		{
			sprintf(str, "%ldcM", rs232radio.band);
			u8g_DrawStr(&u8g, 40, 22,str );
		}
		u8g_DrawStr(&u8g, 2, 32,"Mode:" );
	
		u8g_DrawStr(&u8g, 40, 32,rs232radio.mode);

	
	
	}
	
} while ( u8g_NextPage(&u8g) );	
	
}


double degree_to_radian(double degree)
{
	double radian;
	radian=(degree*(0.0174));
	return radian;
}

int hr_x_coordinate_finder(uint16_t degree,uint16_t radius)
{
	int xcoordinate,intermediate;
	double radian;
	radian= degree_to_radian(degree+90);
	intermediate= radius*(cos(radian));
	xcoordinate= (32-intermediate);
	return xcoordinate;
}
double status_degree;
int hr_y_coordinate_finder(uint16_t degree,uint16_t radius)
{
	int ycoordinate;
	int intermediate;
	double radian;
	radian= degree_to_radian(degree+92);
	intermediate= radius*(sin(radian));
	ycoordinate= (32-intermediate);
	return ycoordinate;
}


void draw_angel_circle(int angel, int type )
{
	int size_a =26;
	if (type == 1)
	{
		size_a =12;
	}
	
	u8g_DrawCircle(&u8g, 30, 30, size_a+1,U8G_DRAW_ALL);
	// rotorns peklinje
	int x_rad= hr_x_coordinate_finder(angel,(size_a-2));
	int y_rad= hr_y_coordinate_finder(angel,(size_a-2));
	u8g_DrawLine(&u8g, x_rad, y_rad,  32,  32);
	// text radie
	
	// target / heading
	if (type == 0)
	{
	char str[4];
	sprintf(str, "%i°",angel);

	u8g_DrawStr(&u8g, 1, 1, str);
	}
	
	
}



void setUpSerial_main()
{
	 // uart port E
	PORTE_OUTSET = PIN3_bm; //Let's make PC7 as TX
	PORTE_DIRSET = PIN3_bm; //TX pin as output
     
	PORTE_OUTCLR = PIN2_bm;
	PORTE_DIRCLR = PIN2_bm; //PC6 as RX
 
    // Baud rate selection
    // BSEL = (32000000 / (2^0 * 16*9600) -1 = 207.333 -> BSCALE = 0
    // FBAUD = ( (32000000)/(2^0*16(207+1)) = 9615.384 -> it's alright
     
    USARTE0_BAUDCTRLB = 0; //Just to be sure that BSCALE is 0
    USARTE0_BAUDCTRLA = 0xCF; // 207
     
     
    
	//8 data bits, no parity and 1 stop bit 
    USARTE0_CTRLC = USART_CHSIZE_8BIT_gc;
     
    //Enable receive and transmit
    USARTE0_CTRLB = USART_TXEN_bm | USART_RXEN_bm; // And enable high speed mode
	USARTE0_CTRLA|= USART_RXCINTLVL_LO_gc;
	
	// usart port F0
	
	PORTF_OUTSET = PIN3_bm; //Let's make PC7 as TX
	PORTF_DIRSET = PIN3_bm; //TX pin as output
     
	PORTF_OUTCLR = PIN2_bm;
	PORTF_DIRCLR = PIN2_bm; //PC6 as RX
 
    // Baud rate selection
    // BSEL = (32000000 / (2^0 * 16*9600) -1 = 207.333 -> BSCALE = 0
    // FBAUD = ( (32000000)/(2^0*16(207+1)) = 9615.384 -> it's alright
     
    USARTF0_BAUDCTRLB = 0; //Just to be sure that BSCALE is 0
    USARTF0_BAUDCTRLA = 0xCF; // 207
     
     
    
	//8 data bits, no parity and 1 stop bit 
    USARTF0_CTRLC = USART_CHSIZE_8BIT_gc;
     
    //Enable receive and transmit
    USARTF0_CTRLB = USART_TXEN_bm | USART_RXEN_bm; // And enable high speed mode
	USARTF0_CTRLA|= USART_RXCINTLVL_LO_gc;
	
	
}

void setUpSerial_rpt()
{

     // uart port c
	PORTC_OUTSET = PIN3_bm; //Let's make PC3 as TX
	PORTC_DIRSET = PIN3_bm; //TX pin as output
	PORTC_OUTCLR = PIN2_bm;
	PORTC_DIRCLR = PIN2_bm; //PC2 as RX	
	bool twoStopBits =1;
    // Baud rate selection
    // BSEL = (32000000 / (2^0 * 16*9600) -1 = 207.333 -> BSCALE = 0
    // FBAUD = ( (32000000)/(2^0*16(207+1)) = 9615.384 -> it's alright
     
    USARTC0_BAUDCTRLB = rs232radio.rs232_prescale; //Just to be sure that BSCALE is 0
    USARTC0_BAUDCTRLA = rs232radio.radio_rs232; // 207
     
    //8 data bits, no parity and 2 stop bit 
    USARTC0_CTRLC = USART_CHSIZE_8BIT_gc |(twoStopBits ? USART_SBMODE_bm : 0);
     
    //Enable receive and transmit
    USARTC0_CTRLB = USART_TXEN_bm | USART_RXEN_bm; // And enable high speed mode
	USARTC0_CTRLA|= USART_RXCINTLVL_HI_gc;
	
	// port d init
	PORTD_OUTSET = PIN3_bm; //Let's make PC7 as TX
	PORTD_DIRSET = PIN3_bm; //TX pin as output
     
	PORTD_OUTCLR = PIN2_bm;
	PORTD_DIRCLR = PIN2_bm; //PC6 as RX
	

    // Baud rate selection
    // BSEL = (32000000 / (2^0 * 16*9600) -1 = 207.333 -> BSCALE = 0
    // FBAUD = ( (32000000)/(2^0*16(207+1)) = 9615.384 -> it's alright
     
    USARTD0_BAUDCTRLB = rs232radio.rs232_prescale; //Just to be sure that BSCALE is 0
    USARTD0_BAUDCTRLA = rs232radio.radio_rs232; // 207
     
     
    
	//8 data bits, no parity and 2 stop bit 
    USARTD0_CTRLC = USART_CHSIZE_8BIT_gc | (twoStopBits ? USART_SBMODE_bm : 0);
     
    //Enable receive and transmit
    USARTD0_CTRLB = USART_TXEN_bm | USART_RXEN_bm; // And enable high speed mode
	USARTD0_CTRLA|= USART_RXCINTLVL_HI_gc;
	

}
   

void draw(void)
{
	u8g_SetFont(&u8g, u8g_font_04b_24);
	char filename[20];
	char filetext[200];

	strcpy( filename, "test.txt" );
	sd_file_open(filename);
	sd_file_read(filetext);
	sd_file_close();
	u8g_DrawRFrame(&u8g, 1, 1, 126, 62, 2);
	u8g_DrawStr(&u8g, 2, 10, "Filename:1");
	u8g_DrawStr(&u8g, 35, 10, filename);
	u8g_DrawStr(&u8g, 2, 20, "Content:");
	u8g_DrawStr(&u8g, 2, 30, filetext);
	

}

void sendChar(char c)
{
	
	while( !(USARTD0_STATUS & USART_DREIF_bm) ); //Wait until DATA buffer is empty
	
	USARTD0_DATA = c;
	
}
void sendChar_c(char c)
{
	
	while( !(USARTC0_STATUS & USART_DREIF_bm) ); //Wait until DATA buffer is empty
	
	USARTC0_DATA =c;
	
	
}
void sendChar_d(char c)
{
	
	while( !(USARTD0_STATUS & USART_DREIF_bm) ); //Wait until DATA buffer is empty	
	USARTD0_DATA =c;
	
}

void sendChar_f(char c)
{
	
	while( !(USARTF0_STATUS & USART_DREIF_bm) ); //Wait until DATA buffer is empty
	
	USARTF0_DATA =c;
	
	
}

void sendString(char *text)
{
	while(*text)
	{
		sendChar(*text++);

	}
}
void sendString_c(char *text)
{
	while(*text)
	{
		sendChar_c(*text++);

	}
}
void sendString_d(char *text)
{
	while(*text)
	{
		sendChar_d(*text++);

	}
}
void sendString_f(char *text)
{
	while(*text)
	{
		sendChar_f(*text++);

	}
}


static int uart_putchar (char c, FILE *stream)
{
    if (c == '\n')
    uart_putchar('\r', stream);
     
    // Wait for the transmit buffer to be empty
    while (  !(USARTE0_STATUS & USART_DREIF_bm) );
     
    // Put our character into the transmit buffer
    USARTE0_DATA = c;
	
	
    return 0;
}

int uart_getchar(FILE *stream)
{
    while( !(USARTE0_STATUS & USART_RXCIF_bm) ); //Wait until data has been received.
    char data = USARTE0_DATA; //Temporarly store received data
    if(data == '\r')
        data = '\n';    
    uart_putchar(data, stream); //Send to console what has been received, so we can see when typing
    return data;        
}
// rotor process
int AZ=0;
int EL =0;
/*
Funtion för easycom samt yeasu format för rotor med återkoppling
*/
void easy_com_angel(volatile unsigned char  * data_in)
{
	char print_str[15];

	// easy com
	if (data_in[0] =='A' && data_in[1] =='Z')
	{
		if(data_in[4] != '.')
		{
			AZ = ((data_in[2]-'0')*100)+((data_in[3]-'0')*10)+(data_in[4]-'0');
			EL = ((data_in[10]-'0')*10)+((data_in[11]-'0'));
		}
		else
		{
			AZ = ((data_in[2]-'0')*10)+((data_in[3]-'0'));
			EL = ((data_in[9]-'0')*10)+((data_in[10]-'0'));
			
		}

/*		printf("AZ %i  EL %i\r",AZ,EL);	*/
		rotor_target  = AZ;
	}
	// yeasu
	if (data_in[0] =='W')
	{
		AZ = ((data_in[1]-'0')*100)+((data_in[2]-'0')*10)+(data_in[3]-'0');
		EL = ((data_in[6]-'0')*10)+((data_in[7]-'0'));
		rotor_target  = AZ;
	}
	if (data_in[0] =='C' && data_in[1] =='2')
	{
		if (rotor_curent  <100)
		{
			//printf("AZ=0%i EL=0%i\r",rad_test,EL);
			sprintf(print_str, "AZ=0%i EL=0%i\r", rotor_curent ,EL);
			
		}
		else
		{
			//printf("AZ=%i EL=0%i\r",rad_test,EL);
			sprintf(print_str, "AZ=%i EL=0%i\r", rotor_curent ,EL);
		
		}

		//printf(print_str);
		sendString_f(print_str);
	// SEND AZ ALONE
	}
	 if(data_in[0] =='C' &&  data_in[1] !='2') 
	{
		if (rotor_curent  <100)
		{
			//printf("AZ=0%i\r",rad_test);
			sprintf(print_str, "AZ=0%i\r", rotor_curent);
		}
		else
		{
			
			sprintf(print_str, "AZ=%i\r", rotor_curent);
			
		}
		sendString_f(print_str);
	}
	if(data_in[0] =='B')
	{
		// send EL yeasu
		//printf("EL=0%i",EL);
		sprintf(print_str, "EL=0%i\r", EL);
		sendString_f(print_str);
	}
	// skriver ut i debugen inkommande data
	//printf(data_in);
	
}



void get_band()
{
	// ft-857d paket
	// get band from freqvensy yeasu
	//printf("%02X%02X\n",message_que.tx[0],message_que.tx[4]);
	
	switch(rs232radio.radio_mode)
	{
		case 0x00:
			rs232radio.mode ="LSB";
			rs232radio.mode_id=0;
			break;
		case 0x01:
			rs232radio.mode ="USB";
			rs232radio.mode_id=1;
			break;
		case 0x02:
			rs232radio.mode ="CW";
			rs232radio.mode_id=2;
			break;
		case 0x03:
			rs232radio.mode ="CW-R";
			rs232radio.mode_id=3;
			break;
		case 0x4:
			rs232radio.mode ="AM";
			rs232radio.mode_id=4;
			break;
		case 0x82:
			rs232radio.mode ="CW-N";
			rs232radio.mode_id=5;
			break;
		case 0x06:
			rs232radio.mode ="WFM";
			rs232radio.mode_id=6;
			break;
		case 0x08:
			rs232radio.mode_id=7;
			rs232radio.mode ="FM";
			break;
		case 0x88:
			rs232radio.mode_id=8;
			rs232radio.mode ="NFM";
			break;
					
		case 0x0A:
			rs232radio.mode_id=9;
			rs232radio.mode ="DIG";
			break;
					
		case 0x0C:
			rs232radio.mode_id=10;
			rs232radio.mode ="PKT";
			break;			
		default:
			rs232radio.mode ="";
			break;

	}
	float band = ((long)30000000 /rs232radio.freqvensy);		
	// sparar frekvensen i struckt
	if (band >-1 && band < 800)
	{			
		if((float)band <1)
		{
			rs232radio.meter=0;
			rs232radio.band = 70;
		}
		else
		{
			rs232radio.meter=1;
			rs232radio.band = band;
		}
	}
	
}



char line[20];
int  al = 0;
// main interupt read


volatile unsigned char data_rotor_in[20];

int data_count=0;
ISR(USARTF0_RXC_vect)
{
    // Get data from the USART in register
		data_rotor_in[data_count] = USARTF0_DATA;
	
		// End of line!
		if (data_rotor_in[data_count] == '\r') {
			data_count = 0;
			easy_com_angel(data_rotor_in);
			// Reset to 0, ready to go again
        
		} else {
			data_count++;
		}

}



// interupt rotar encoder
// rotary encoder
ISR(PORTH_INT0_vect)
{
 if (!(PORTH.IN & PIN1_bm))
 {
	  meny_selected--;
 }else
 {
	  meny_selected++;
 }
 _delay_us(100);

}

// rotary encoder
 ISR(PORTH_INT1_vect)
{

}

ISR(PORTD_INT0_vect)
{
	new_can_message =1;
}
ISR(PORTA_INT0_vect)
{
	if((PORTA.IN & PIN0_bm) ==0 )
	{
		rs232radio.ptt =1;
	}
	else
		rs232radio.ptt =0;
	
}