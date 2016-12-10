/*
 * io_driver.h
 *
 * Created: 2016-10-06 00:57:27
 *  Author: peter
 */ 

#ifndef iodriver
#define iodriver

/***
function to get the select button value
*/

int Select_buttion();

/***
function to get the select button1
*/
int buttion_one();

/***
function to get the select button2
*/
int buttion_two();

/***
function to get the select button3
*/
int buttion_three();

/***
function to get the select four
*/
int buttion_four();

/***
function to get the select five
*/
int buttion_five();

/***
function to get the ptt from radio i aux port
*/
int Radio_ptt();

void setup_buttons();

void  button_test();

void toogle_alert();

void clear_alert();


#endif
