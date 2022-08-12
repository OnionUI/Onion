#ifndef MENU_H__
#define MENU_H__

#include <string.h>
#include <stdlib.h>

typedef enum {LIST_SMALL, LIST_LARGE} list_type_e;

typedef struct ListItem
{
	char label[STR_MAX];
	char description[STR_MAX];
	char payload[STR_MAX];
	int action;
	bool is_toggle;
	bool state;
} ListItem;

typedef struct List
{
	int item_count;
	int active_pos;
	int scroll_pos;
	int scroll_height;
	list_type_e list_type;
	ListItem* items;
} List;

List list_create(int max_items, list_type_e list_type)
{
	List menu = {
		.scroll_height = list_type == LIST_SMALL ? 6 : 4,
		.list_type = list_type,
		.items = (ListItem*)malloc(sizeof(ListItem) * max_items)
	};
	return menu;
}

void list_addItem(List* list, ListItem item)
{
	list->items[list->item_count++] = item;
}

void list_scroll(List* list)
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

void list_moveUp(List* list, bool key_repeat)
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
	list_scroll(list);
}

void list_moveDown(List* list, bool key_repeat)
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
	list_scroll(list);
}

ListItem* list_applyAction(List* list)
{
	ListItem *item = &list->items[list->active_pos];

	if (item->is_toggle)
		item->state = !item->state;

	return item;
}

bool list_getToggle(List* list, int action)
{
	int i;
	ListItem* item;

	for (i = 0; i < list->item_count; i++) {
		item = &list->items[i];
		if (item->action == action)
			return item->state;
	}

	return false;
}

void list_free(List* list)
{
	free(list->items);
}

#endif // MENU_H__
