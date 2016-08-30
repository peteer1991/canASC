#include <avr/pgmspace.h>

#define Menu_MAX 7
typedef void (*function_pointer);
//typedef void FUNC(struct MENU_ITEMS * men);

typedef struct MENU_ITEMS
{
	char Text;
	char _MENU_NAMES[Menu_MAX];
	int SIZE;
} MENU_ITEMS;

struct menu_contoller
{
	int  *menu_size;
	int  *slector_at ;
	int  *enabled;
}me;

