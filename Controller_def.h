
/*
 * Controller_def.h
 *
 * Created: 2015-06-23 13:41:29
 *  Author: peter
 */ 

typedef struct 
{
	int band;
	int switch_id;
	int Rotor_id;
	int Amp_id;
	int Power;
} cont;

typedef struct 
{
	int taget;
	int curent;

} pos_rot;
typedef struct
{
	int radio_rs232;
	int rs232_prescale;
	char * model;
	uint32_t band;
	int meter;
	int enable;
	int ptt;
	int mode_id;
	int amp_id;
	char * freq;
	long freqvensy;
	int radio_mode;
	char * mode;

}radio;

typedef struct
{
	char rx[6],tx[6],out_f[5],out_tx[2];
	int dest;

	int max_count;
	char curent_message;
	int curent_lock;
	
} rs232message;

typedef struct
{
	uint32_t band;
	int ptt;
	int amp_id;
	int tx_ready;
	int power_fwd;
	int power_rev;
	int power_max;
	}Amp ;