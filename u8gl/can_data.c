/*
 * can_data.c
 *
 * Created: 2014-12-13 22:28:55
 *  Author: peter
 */ 
#include <avr/io.h>

#include <string.h>
#include <util/delay.h>
#include <stdio.h>

extern void CAN_message_send();
extern int CAN_int_flag;
extern void CAN_data_receive();
int  Max_units = 255;
int  Master_unit_id = 1;

uint8_t Recived_units[255];

typedef struct{
	uint8_t msg_id;
	uint8_t data_length;
	uint8_t data[8];
}can_message_t;

// skannar alla 255 adresser på canbusen 
void Scan_canbuss(can_message_t message_recive)
{
	can_message_t message_handeling;
	// send to all canbus units
	for(int i=0;i<Max_units;i++)
	{
		message_handeling.msg_id=i;
		// sending comand for scan 
		message_handeling.data[0] = 100;
		message_handeling.data[1] = Master_unit_id;
		CAN_message_send(&message_handeling);

	}
}
void Scan_recive(can_message_t message_recive)
{
	if(message_recive.msg_id == 205)
	{
		printf("ID=%d,%d\r",message_recive.msg_id,message_recive.data[1]);
	}
}
void Can_get_identity(can_message_t message_recive)
{

if(message_recive.msg_id == 200 && message_recive.data[0] == 100)
{

	Recived_units[message_recive.data[1]] =1;
}

}
void Get_number_of_units()
{
	//if(calc_array(0,50) >0)
	printf("Antal Slutsteg : %d \r",calc_array(0,50));
	//if(calc_array(50,100) >0)
	printf("Antal Switchar : %d \r",calc_array(50,100));
	//if(calc_array(100,150) >0)
	printf("Antal Rotorer : %d \r",calc_array(100,150));

}
int count_amp()
{
	return calc_array(0,50);
}
int count_sw()
{
	return calc_array(50,100);
}
int count_rot()
{
	return calc_array(100,150);
}

int calc_array(int start,int end_val )
{
	int calc_val =0;
	
	for ( int i=start;i<end_val;i++)
	{
		if(Recived_units[i] ==1)
		{
			calc_val++;
			
		}
	}
	
	//printf("start, %d end,%d , val %d\r",start,end_val,calc_val);
	return calc_val;
}




void print_scan_to_usart(){
	
	// loopar geronom canbuss
	for(int i=0;i<Max_units;i++)
	{
		printf("ID=%d,%d\r",i,Recived_units[i]);
	}
	
}

void Send_Position_to_rotor(int rotor_id,int position)
{
			can_message_t message_handeling;
			message_handeling.msg_id = 100;
			message_handeling.data[0] = rotor_id;
			message_handeling.data[1] =position >>8;
			message_handeling.data[2] =position & 0xFF;
			message_handeling.data_length =3;
			CAN_message_send(&message_handeling);
	
}