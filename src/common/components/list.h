#ifndef COMPONENT_LIST_H__
#define COMPONENT_LIST_H__

#include <stdbool.h>

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

List list_create(int max_items, ListType list_type);
List list_createWithTitle(int max_items, ListType list_type, const char *title);
List list_createWithSticky(int max_items, const char *title);

ListItem *list_addItem(List *list, ListItem item);
ListItem *list_addItemWithInfoNote(List *list, ListItem item, const char *info_note);

ListItem *list_currentItem(List *list);

void list_updateStickyNote(ListItem *item, const char *message);
const char *list_getStickyNote(ListItem *item);

bool list_scrollTo(List *list, int active_pos);

bool list_keyUp(List *list, bool key_repeat);
bool list_keyDown(List *list, bool key_repeat);
bool list_keyLeft(List *list, bool key_repeat);
bool list_keyRight(List *list, bool key_repeat);

bool list_activateItem(List *list);
bool list_resetCurrentItem(List *list);
bool list_hasInfoNote(List *list);

void list_getItemValueLabel(ListItem *item, char *out_label);
void list_sortByLabel(List *list);

void list_free(List *list);

#endif // COMPONENT_LIST_H__
