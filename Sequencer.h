/*
 * Sequencer.c
 *
 * Created: 2016-12-09 21:48:58
 *  Author: peter
 */ 

void TX_sequens();
void RX_sequens();
void radio_start_tx();
void start_preeamp();
void stop_preamp();
void start_amp();
void stop_amp();
void radio_inhibit(int state);
void preamp_ready();
void amp_ready();
void set_amp_id(int amp_id);
void Set_preamp_id(int preamp_id);
int get_amp_id();
