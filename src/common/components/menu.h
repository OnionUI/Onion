#ifndef MENU_H__
#define MENU_H__

#include <string.h>
#include <stdlib.h>

#define LABEL_SIZE 128

typedef struct MenuItem
{
	char label[LABEL_SIZE];
	int action;
	bool is_toggle;
	bool state;
} MenuItem_s;

typedef struct Menu
{
	int item_count;
	int active_pos;
	int scroll_pos;
	int scroll_height;
	MenuItem_s* items;
} Menu_s;

Menu_s menu_create(int max_items)
{
	Menu_s menu = {
		.items = (MenuItem_s*)malloc(sizeof(MenuItem_s) * max_items),
		.scroll_height = 6
	};
	return menu;
}

void menu_addItem(Menu_s* menu, char* label, int action)
{
	MenuItem_s item = {.action = action};
	strncpy(item.label, label, LABEL_SIZE-1);
	menu->items[menu->item_count++] = item;
}

void menu_addToggle(Menu_s* menu, char* label, int action, bool state)
{
	MenuItem_s item = {.action = action, .is_toggle = true, .state = state};
	strncpy(item.label, label, LABEL_SIZE-1);
	menu->items[menu->item_count++] = item;
}

void menu_scroll(Menu_s* menu)
{
	// Scroll up
	if (menu->active_pos < menu->scroll_pos)
		menu->scroll_pos = menu->active_pos;
	// Scroll down
	else if (menu->active_pos >= menu->scroll_pos + menu->scroll_height)
		menu->scroll_pos = menu->active_pos - menu->scroll_height + 1;

	// No scrolling if not enough items
	if (menu->item_count <= menu->scroll_height)
		menu->scroll_pos = 0;
	// Max scroll to last item
	else if (menu->scroll_pos + menu->scroll_height > menu->item_count)
		menu->scroll_pos = menu->item_count - menu->scroll_height;
}

void menu_moveUp(Menu_s* menu, bool key_repeat)
{
	// Wrap-around (move to bottom)
	if (menu->active_pos == 0) {
		if (key_repeat)
			return;
		menu->active_pos = menu->item_count - 1;
	}
	// Descrease selection (move up)
	else
		menu->active_pos -= 1;
	menu_scroll(menu);
}

void menu_moveDown(Menu_s* menu, bool key_repeat)
{
	// Wrap-around (move to top)
	if (menu->active_pos == menu->item_count - 1) {
		if (key_repeat)
			return;
		menu->active_pos = 0;
	}
	// Increase selection (move down)
	else
		menu->active_pos += 1;
	menu_scroll(menu);
}

int menu_applyAction(Menu_s* menu)
{
	MenuItem_s* item = &menu->items[menu->active_pos];

	if (item->is_toggle)
		item->state = !item->state;

	return item->action;
}

bool menu_getToggle(Menu_s* menu, int action)
{
	int i;
	MenuItem_s* item;

	for (i = 0; i < menu->item_count; i++) {
		item = &menu->items[i];
		if (item->action == action)
			return item->state;
	}

	return false;
}

void menu_free(Menu_s* menu)
{
	free(menu->items);
}

#endif // MENU_H__
