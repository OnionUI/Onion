#ifndef MENU_H__
#define MENU_H__

#include <string.h>
#include <stdlib.h>

#define MAX_NUM_VALUES 100

typedef enum list_type {LIST_SMALL, LIST_LARGE} ListType;
typedef enum item_type {ACTION, TOGGLE, MULTIVALUE} ListItemType;

typedef struct ListItem
{
	int _id;
	ListItemType item_type;
	char label[STR_MAX];
	char description[STR_MAX];
	char payload[STR_MAX];
	int value;
	int value_max;
	char value_labels[MAX_NUM_VALUES][STR_MAX];
	void (*value_formatter)(void *self, char *out_label);
	void (*action)(void *self);
	int action_id;
	int _reset_value;
	void *icon_ptr;
} ListItem;

typedef struct List
{
	char title[STR_MAX];
	int item_count;
	int active_pos;
	int scroll_pos;
	int scroll_height;
	ListType list_type;
	ListItem* items;
	bool _created;
} List;

List list_create(int max_items, ListType list_type)
{
	return (List){
		.scroll_height = list_type == LIST_SMALL ? 6 : 4,
		.list_type = list_type,
		.items = (ListItem*)malloc(sizeof(ListItem) * max_items),
		._created = true
	};
}

void list_addItem(List *list, ListItem item)
{
	item._reset_value = item.value;
	item._id = list->item_count;
	list->items[item._id] = item;
	list->item_count++;
}

ListItem* list_currentItem(List *list)
{
	if (list->active_pos >= list->item_count)
		return NULL;
	return &list->items[list->active_pos];
}

void list_scroll(List *list)
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

bool list_keyUp(List *list, bool key_repeat)
{
	// Wrap-around (move to bottom)
	if (list->active_pos == 0) {
		if (key_repeat)
			return false;
		list->active_pos = list->item_count - 1;
	}
	// Descrease selection (move up)
	else
		list->active_pos -= 1;
	list_scroll(list);

	return true;
}

bool list_keyDown(List *list, bool key_repeat)
{
	// Wrap-around (move to top)
	if (list->active_pos == list->item_count - 1) {
		if (key_repeat)
			return false;
		list->active_pos = 0;
	}
	// Increase selection (move down)
	else
		list->active_pos += 1;
	list_scroll(list);

	return true;
}

bool list_keyLeft(List *list, bool key_repeat)
{
	bool apply_action = false;
	ListItem *item = list_currentItem(list);

	if (item == NULL)
		return false;

	int old_value = item->value;

	switch (item->item_type) {
		case TOGGLE:
			if (item->value != 0) {
				item->value = 0;
				apply_action = true;
			}
			break;
		case MULTIVALUE:
			if (item->value == 0) {
				if (!key_repeat)
					item->value = item->value_max;
			}
			else item->value--;
			apply_action = true;
			break;
		default: break;
	}

	if (apply_action && item->action != NULL)
		item->action((void*)item);

	return old_value != item->value;
}

bool list_keyRight(List *list, bool key_repeat)
{
	bool apply_action = false;
	ListItem *item = list_currentItem(list);

	if (item == NULL)
		return false;

	int old_value = item->value;

	switch (item->item_type) {
		case TOGGLE:
			if (item->value != 1) {
				item->value = 1;
				apply_action = true;
			}
			break;
		case MULTIVALUE:
			if (item->value == item->value_max) {
				if (!key_repeat)
					item->value = 0;
			}
			else item->value++;
			apply_action = true;
			break;
		default: break;
	}

	if (apply_action && item->action != NULL)
		item->action((void*)item);

	return old_value != item->value;
}

bool list_activateItem(List *list)
{
	ListItem *item = list_currentItem(list);

	if (item == NULL)
		return false;

	int old_value = item->value;

	switch (item->item_type) {
		case TOGGLE: item->value = !item->value; break;
		case MULTIVALUE:
			if (item->value == item->value_max)
				item->value = 0;
			else item->value++;
			break;
		default: break;
	}

	if (item->action != NULL)
		item->action((void*)item);

	return old_value != item->value;
}

bool list_resetCurrentItem(List *list)
{
	ListItem *item = list_currentItem(list);

	if (item == NULL || item->value == item->_reset_value)
		return false;

	item->value = item->_reset_value;

	if (item->action != NULL)
		item->action((void*)item);

	return true;
}

void list_getItemValueLabel(ListItem *item, char *out_label)
{
	if (item->value_formatter != NULL)
		item->value_formatter(item, out_label);
	else if (item->value_labels[0][0] != '\0')
		sprintf(out_label, "%s", item->value_labels[item->value]);
	else
		sprintf(out_label, "%d", item->value);
}

void list_free(List *list)
{
	if (!list->_created)
		return;
	for (int i = 0; i < list->item_count; i++) {
		ListItem *item = &list->items[i];
		if (item->icon_ptr != NULL)
			SDL_FreeSurface((SDL_Surface*)item->icon_ptr);
	}
	free(list->items);
	list->_created = false;
}

#endif // MENU_H__
