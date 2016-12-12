/*
 * menus.h
 *
 * Created: 2015-06-23 01:49:56
 *  Author: peter
 */ 
#include "u8g/u8g.h"

typedef void (*function_menu)();

struct Menu {
	int current;
	function_menu main;
} menu;

