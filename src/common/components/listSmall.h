#ifndef MENU_H__
#define MENU_H__

#include <string.h>
#include <stdlib.h>

#define LABEL_SIZE 128

typedef struct ListSmallItem
{
	char label[LABEL_SIZE];
	int action;
	bool is_toggle;
	bool state;
} ListSmallItem;

typedef struct ListSmall
{
	int item_count;
	int active_pos;
	int scroll_pos;
	int scroll_height;
	ListSmallItem* items;
} ListSmall;

ListSmall listSmall_create(int max_items)
{
	ListSmall menu = {
		.items = (ListSmallItem*)malloc(sizeof(ListSmallItem) * max_items),
		.scroll_height = 6
	};
	return menu;
}

void listSmall_addItem(ListSmall* list, char* label, int action)
{
	ListSmallItem item = {.action = action};
	strncpy(item.label, label, LABEL_SIZE-1);
	list->items[list->item_count++] = item;
}

void listSmall_addToggle(ListSmall* list, char* label, int action, bool state)
{
	ListSmallItem item = {.action = action, .is_toggle = true, .state = state};
	strncpy(item.label, label, LABEL_SIZE-1);
	list->items[list->item_count++] = item;
}

void listSmall_scroll(ListSmall* list)
{
	// Scroll up
	if (list->active_pos < list->scroll_pos)
		list->scroll_pos = list->active_pos;
	// Scroll down
	else if (list->active_pos >= list->scroll_pos + list->scroll_height)
		list->scroll_pos = list->active_pos - list->scroll_height + 1;

	// No scrolling if not enough items
	if (list->item_count <= list->scroll_height)
		list->scroll_pos = 0;
	// Max scroll to last item
	else if (list->scroll_pos + list->scroll_height > list->item_count)
		list->scroll_pos = list->item_count - list->scroll_height;
}

void listSmall_moveUp(ListSmall* list, bool key_repeat)
{
	// Wrap-around (move to bottom)
	if (list->active_pos == 0) {
		if (key_repeat)
			return;
		list->active_pos = list->item_count - 1;
	}
	// Descrease selection (move up)
	else
		list->active_pos -= 1;
	listSmall_scroll(list);
}

void listSmall_moveDown(ListSmall* list, bool key_repeat)
{
	// Wrap-around (move to top)
	if (list->active_pos == list->item_count - 1) {
		if (key_repeat)
			return;
		list->active_pos = 0;
	}
	// Increase selection (move down)
	else
		list->active_pos += 1;
	listSmall_scroll(list);
}

int listSmall_applyAction(ListSmall* list)
{
	ListSmallItem* item = &list->items[list->active_pos];

	if (item->is_toggle)
		item->state = !item->state;

	return item->action;
}

bool listSmall_getToggle(ListSmall* list, int action)
{
	int i;
	ListSmallItem* item;

	for (i = 0; i < list->item_count; i++) {
		item = &list->items[i];
		if (item->action == action)
			return item->state;
	}

	return false;
}

void listSmall_free(ListSmall* list)
{
	free(list->items);
}

#endif // MENU_H__
