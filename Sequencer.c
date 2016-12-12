/*
 * Sequencer.c
 *
 * Created: 2016-12-09 21:48:58
 *  Author: peter
 */ 
#include <avr/io.h>
#include "Controller_def.h";
#include "Sequencer.h";
#include "can/can.h"
#define  Debug_seq 1
seqvenser Seqvenser;
wait_for_struckt wait_for;

void TX_sequens()
{
	radio_inhibit(1);
	
	if (Seqvenser.preamp >0)
	{
		stop_preamp();
	}
	else
	{
		wait_for.preamp=0;
	}
	
	if (wait_for.preamp == 0)
	{
			// if amp id sent amp to trasmit mode;
			if (Seqvenser.Amp_id >0)
			{
				start_amp();
				
			}
	}
	
	if ((wait_for.preamp == 0) && (wait_for.amplifier == 0))
	{
		radio_inhibit(0);
			if (Debug_seq == 1 )
			{
				printf("Inhibit off\n");
			}
		
	}
	
}
void RX_sequens()
{
	if (Seqvenser.Amp_id >0)
	{
		stop_amp();
		
	}
	if ((wait_for.preamp == 0) && (wait_for.amplifier == 0))
	{
		
			if (Seqvenser.Amp_id >0)
			{
				start_preeamp();
				
			}
	}
	


}
void radio_start_tx()
{
	// Enable radio inhibit pin
	radio_inhibit(1);
}

void start_preeamp()
{
	if (Debug_seq == 1 )
	{
		printf("Start preamp with id %i \n",Seqvenser.Amp_id);
	}
	wait_for.preamp =0;
}
void stop_preamp()
{
	if (Debug_seq == 1 )
	{
		printf("Stop preamp with id %i \n",Seqvenser.Amp_id);
	}
	wait_for.preamp =1;
}
void start_amp()
{
	if (Debug_seq == 1 )
	{
		printf("Start amp with id %i \n",Seqvenser.Amp_id);
	}
	
	wait_for.preamp =1;
	
	can_message_t send;
	send.msg_id=1;
	send.data_length=2;
	send.data[0]=Seqvenser.Amp_id;
	send.data[1]=1;
	can_queue_Enqueue(send);
	_delay_ms(5);
	
	
}
void stop_amp()
{
	if (Debug_seq == 1 )
	{
		printf("Stop amp with id %i \n",Seqvenser.Amp_id);
	}

		can_message_t send;
		send.msg_id=1;
		send.data_length=2;
		send.data[0]=Seqvenser.Amp_id;
		send.data[1]=2;
		can_queue_Enqueue(send);
		_delay_ms(5);
		
		wait_for.amplifier=1;
		
		
}
void radio_inhibit(int state)
{
	if (state == 1)
	{
		if (Debug_seq == 1 )
		{
			printf("Start inhibit\n");
		}
		PORTA.OUTSET = PIN2_bm;
	}
	else
	{
		PORTA.OUTCLR = PIN2_bm;
	}
		
}
void preamp_ready()
{
	wait_for.preamp =0;
}
void amp_ready()
{
	wait_for.amplifier =0;
}
void set_amp_id(int amp_id)
{
	Seqvenser.Amp_id = amp_id;
}
void Set_preamp_id(int preamp_id)
{
	Seqvenser.preamp = preamp_id;
}
int get_amp_id()
{
	return Seqvenser.Amp_id;
}