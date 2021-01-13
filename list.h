#include <stdlib.h>

struct node_t;

typedef struct node_t
{
	void* data;
	struct node_t* next;
	struct node_t* prev;
} node_t;

typedef struct list_t
{
	node_t* head;
	node_t* tail;
	size_t length;
} list_t;


list_t* list_new();

void list_append(list_t* list, void* data);
void list_prepend(list_t* list, void* data);
int list_insert(list_t* list, void* data, size_t idx);
void* list_get(list_t* list, size_t idx);
void list_remove(list_t* list, size_t idx);
void list_remove_node(list_t* list, node_t* node);

void* list_find(list_t* list, void* comp, int (*func)(void* comp, void* cur));

void list_free(list_t* list);
