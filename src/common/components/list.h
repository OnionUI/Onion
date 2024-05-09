#ifndef MENU_H__
#define MENU_H__

#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "utils/str.h"

#define MAX_NUM_VALUES 100

typedef enum list_type { LIST_SMALL,
                         LIST_LARGE } ListType;
typedef enum item_type { ACTION,
                         TOGGLE,
                         MULTIVALUE } ListItemType;

typedef struct ListItem {
    int _id;
    ListItemType item_type;
    bool disabled;
    bool show_opaque;
    bool disable_arrows;
    bool disable_a_btn;
    bool alternative_arrow_action;
    char label[STR_MAX];
    char description[STR_MAX];
    char payload[STR_MAX];
    void *payload_ptr;
    int value;
    int value_min;
    int value_max;
    char value_labels[MAX_NUM_VALUES][STR_MAX];
    void (*value_formatter)(void *self, char *out_label);
    void (*action)(void *self);
    void (*arrow_action)(void *self);
    int action_id;
    int _reset_value;
    void *icon_ptr;
    void *preview_ptr;
    char preview_path[STR_MAX];
    char sticky_note[STR_MAX];
    char info_note[STR_MAX];
} ListItem;

typedef struct List {
    char title[STR_MAX];
    int _id;
    int item_count;
    int active_pos;
    int scroll_pos;
    int scroll_height;
    ListType list_type;
    ListItem *items;
    bool has_sticky;
    bool _created;
} List;

static int list_id_incr = 0;

int _list_modulo(int x, int n) { return (x % n + n) % n; }

int list_countVisible(List *list)
{
    int n = 0, i;
    for (i = 0; i < list->item_count; i++) {
        if (!list->items[i].disabled)
            n++;
    }
    return n;
}

void _list_ensureVisible(List *list, int direction, int items_left)
{
    if (list->items[list->active_pos].disabled) {
        list->active_pos = _list_modulo(list->active_pos + direction, list->item_count);
        if (items_left > 0) {
            _list_ensureVisible(list, direction, items_left - 1);
        }
    }
}

void list_ensureVisible(List *list, int direction)
{
    _list_ensureVisible(list, direction, list->item_count);
}

bool _list_did_wraparound(int before, int after, int direction)
{
    int offset = after - before;
    return offset != 0 && (direction > 0) != (offset > 0);
}

ListItem *list_getVisibleItemAt(List *list, int index)
{
    int items_left = list->item_count - index;
    while (list->items[index].disabled && items_left-- > 0) {
        index++;
    }
    return index < list->item_count ? &list->items[index] : NULL;
}

void list_hideAllExcept(List *list, ListItem *item, bool disabled)
{
    for (int i = 0; i < list->item_count; i++) {
        if (i == item->_id)
            continue;
        list->items[i].disabled = disabled;
    }
}

List list_create(int max_items, ListType list_type)
{
    return (List){.scroll_height = list_type == LIST_SMALL ? 6 : 4,
                  .list_type = list_type,
                  .items = (ListItem *)calloc(max_items, sizeof(ListItem)),
                  ._created = true,
                  ._id = list_id_incr++};
}

List list_createWithTitle(int max_items, ListType list_type, const char *title)
{
    List list = list_create(max_items, list_type);
    strncpy(list.title, title, STR_MAX - 1);
    return list;
}

List list_createWithSticky(int max_items, const char *title)
{
    List list = list_createWithTitle(max_items, LIST_SMALL, title);
    list.scroll_height = 5;
    list.has_sticky = true;
    return list;
}

ListItem *list_addItem(List *list, ListItem item)
{
    item._reset_value = item.value;
    item._id = list->item_count;
    memset(item.info_note, 0, STR_MAX);
    list->items[item._id] = item;
    list->item_count++;
    if (item.disabled && list->active_pos == item._id) {
        list->active_pos = item._id + 1;
    }
    return &(list->items[item._id]);
}

ListItem *list_addItemWithInfoNote(List *list, ListItem item, const char *info_note)
{
    ListItem *_item = list_addItem(list, item);
    strcpy(_item->info_note, info_note);
    return _item;
}

ListItem *list_currentItem(List *list)
{
    if (list->active_pos >= list->item_count)
        return NULL;
    return &list->items[list->active_pos];
}

void list_updateStickyNote(ListItem *item, const char *message)
{
    strncpy(item->sticky_note, message, STR_MAX - 1);
}

const char *list_getStickyNote(ListItem *item)
{
    return item->sticky_note;
}

void _list_scroll(List *list, int pos)
{
    pos = _list_modulo(pos, list->item_count);

    // Scroll up
    if (pos < list->scroll_pos)
        list->scroll_pos = pos;
    // Scroll down
    else if (pos >= list->scroll_pos + list->scroll_height)
        list->scroll_pos = pos - list->scroll_height + 1;

    // No scrolling if not enough items
    if (list->item_count <= list->scroll_height)
        list->scroll_pos = 0;
    // Max scroll to last item
    else if (list->scroll_pos + list->scroll_height > list->item_count)
        list->scroll_pos = list->item_count - list->scroll_height;
}

void list_scroll(List *list)
{
    _list_scroll(list, list->active_pos);
}

bool list_scrollTo(List *list, int active_pos)
{
    list->active_pos = _list_modulo(active_pos, list->item_count);
    list_ensureVisible(list, 1);
    list_scroll(list);
    return true;
}

bool list_keyUp(List *list, bool key_repeat)
{
    int old_pos = list->active_pos;

    // Wrap-around (move to bottom)
    if (list->active_pos == 0) {
        if (key_repeat)
            return false;
        list->active_pos = list->item_count - 1;
    }
    // Decrease selection (move up)
    else
        list->active_pos -= 1;

    list_ensureVisible(list, -1);

    if (_list_did_wraparound(old_pos, list->active_pos, -1)) {
        if (list->scroll_pos > 0) {
            _list_scroll(list, list->scroll_pos - 1);
            list->active_pos = old_pos;
        }
        else {
            _list_scroll(list, list->item_count - 1);
        }
    }
    else {
        list_scroll(list);
    }

    return true;
}

bool list_keyDown(List *list, bool key_repeat)
{
    int old_pos = list->active_pos;

    // Wrap-around (move to top)
    if (list->active_pos == list->item_count - 1) {
        if (key_repeat)
            return false;
        list->active_pos = 0;
    }
    // Increase selection (move down)
    else
        list->active_pos += 1;

    list_ensureVisible(list, 1);

    if (_list_did_wraparound(old_pos, list->active_pos, 1)) {
        if (list->scroll_pos < list->item_count - list->scroll_height) {
            _list_scroll(list, list->scroll_pos + list->scroll_height);
            list->active_pos = old_pos;
        }
        else {
            _list_scroll(list, 0);
        }
    }
    else {
        list_scroll(list);
    }

    return true;
}

bool list_keyLeft(List *list, bool key_repeat)
{
    bool apply_action = false;
    ListItem *item = list_currentItem(list);

    if (item == NULL || item->disable_arrows)
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
        if (item->value == item->value_min) {
            if (!key_repeat)
                item->value = item->value_max;
        }
        else
            item->value--;
        apply_action = true;
        break;
    default:
        break;
    }

    if (apply_action) {
        if (item->alternative_arrow_action)
            item->arrow_action((void *)item);
        else if (item->action != NULL)
            item->action((void *)item);
    }

    return old_value != item->value;
}

bool list_keyRight(List *list, bool key_repeat)
{
    bool apply_action = false;
    ListItem *item = list_currentItem(list);

    if (item == NULL || item->disable_arrows)
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
                item->value = item->value_min;
        }
        else
            item->value++;
        apply_action = true;
        break;
    default:
        break;
    }

    if (apply_action) {
        if (item->alternative_arrow_action)
            item->arrow_action((void *)item);
        else if (item->action != NULL)
            item->action((void *)item);
    }

    return old_value != item->value;
}

bool list_activateItem(List *list)
{
    ListItem *item = list_currentItem(list);

    if (item == NULL || item->disable_a_btn)
        return false;

    int old_value = item->value;

    switch (item->item_type) {
    case TOGGLE:
        item->value = !item->value;
        break;
    case MULTIVALUE:
        if (item->value == item->value_max)
            item->value = item->value_min;
        else
            item->value++;
        break;
    default:
        break;
    }

    if (item->action != NULL)
        item->action((void *)item);

    return old_value != item->value;
}

bool list_hasInfoNote(List *list)
{
    ListItem *item = list_currentItem(list);

    if (item == NULL || strlen(item->info_note) == 0)
        return false;

    return true;
}

bool list_resetCurrentItem(List *list)
{
    ListItem *item = list_currentItem(list);

    if (item == NULL || item->value == item->_reset_value)
        return false;

    item->value = item->_reset_value;

    if (item->action != NULL)
        item->action((void *)item);

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
            SDL_FreeSurface((SDL_Surface *)item->icon_ptr);
        if (item->preview_ptr != NULL)
            SDL_FreeSurface((SDL_Surface *)item->preview_ptr);
    }
    free(list->items);
    list->_created = false;
}

int _list_comp_labels(const void *a, const void *b)
{
    return strcasecmp(((ListItem *)a)->label, ((ListItem *)b)->label);
}

void list_sortByLabel(List *list)
{
    qsort(list->items, list->item_count, sizeof(ListItem), _list_comp_labels);
}

#endif // MENU_H__
