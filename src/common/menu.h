#ifndef MENU_H__
#define MENU_H__

#include "./theme.h"

typedef struct MenuItem
{
	char label[128];
	int action;
	bool is_toggle;
	bool state;
} MenuItem_s;

typedef struct Menu
{
	int item_count;
	int current_pos;
	MenuItem_s* items;
} Menu_s;

#endif // MENU_H__
