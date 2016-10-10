/*
 * io_driver.c
 *
 * Created: 2016-10-06 00:43:32
 *  Author: peter
 */
#include <avr/interrupt.h>
#include <avr/io.h>



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
/************************************************************************/
/* fuction to set pullup on buttons           */
/************************************************************************/
void setup_buttons();
void button_test();

// alert led toogle functions
void toogle_alert();
void clear_alert();