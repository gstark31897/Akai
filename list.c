#include "list.h"

// TODO: add custom freer function pointer that can free complex structs

list_t* list_new()
{
	list_t* list = malloc(sizeof(list_t));
	list->head = NULL;
	list->tail = NULL;
	list->length = 0;
	return list;
}

void list_append(list_t* list, void* data)
{
	node_t* item = malloc(sizeof(node_t));
	item->data = data;
	item->next = NULL;
	item->prev = list->tail;
	if (list->tail)
	{
		list->tail->next = item;
	}
	list->tail = item;
	if (!list->head)
	{
		list->head = item;
	}
	++list->length;
}

void list_prepend(list_t* list, void* data)
{
	node_t* item = malloc(sizeof(node_t));
	item->data = data;
	item->next = list->head;
	item->prev = NULL;
	if (list->head)
	{
		list->head->prev = item;
	}
	list->head = item;
	if (!list->tail)
	{
		list->tail = item;
	}
	++list->length;
}

int list_insert(list_t* list, void* data, size_t idx)
{
	if (idx > list->length)
	{
		return 1;
	}

	if (idx == 0)
	{
		list_prepend(list, data);
		return 0;
	}
	else if (idx == list->length)
	{
		list_append(list, data);
	}

	node_t* cur = list->head;
	for (size_t i = 1; i < list->length; ++i)
	{
		cur = cur->next;
	}

	node_t* item = malloc(sizeof(node_t));
	cur->next->prev = item;
	item->next = cur->next;
	cur->next = item;
	item->prev = cur;

	return 0;
}

void* list_get(list_t* list, size_t idx)
{
	if (idx > list->length)
	{
		return NULL;
	}
	else if (idx == 0)
	{
		return list->head->data;
	}
	else if (idx == list->length - 1)
	{
		return list->tail->data;
	}

	node_t* cur = list->head;
	for (size_t i = 0; i < idx; ++i)
	{
		cur = cur->next;
	}
	return cur->data;
}

void list_remove(list_t* list, size_t idx)
{
	if (idx > list->length)
	{
		return;
	}
	else if (idx == 0)
	{
		list->head = list->head->next;
		free(list->head->prev);
		list->head->prev = NULL;
		return;
	}
	else if (idx == list->length - 1)
	{
		list->tail = list->tail->prev;
		free(list->tail->next);
		list->tail->next = NULL;
		return;
	}

	node_t* cur = list->head;
	for (size_t i = 0; i < idx; ++i)
	{
		cur = cur->next;
	}
	node_t* prev = cur->prev;
	node_t* next = cur->next;
	free(cur);
	prev->next = next;
	next->prev = prev;
}

void* list_find(list_t* list, void* comp, int (*func)(void*, void*))
{
	node_t* cur = list->head;
	while (cur)
	{
		if (func(comp, cur->data) == 0)
		{
			return cur->data;
		}
		cur = cur->next;
	}
	return NULL;
}

void list_free(list_t* list)
{
	node_t* cur = list->head;
	for (size_t i = 2; i < list->length; ++i)
	{
		free(cur->data);
		cur = cur->next;
		free(cur->prev);
	}
	if (list->tail)
	{
		free(list->tail->data);
		free(list->tail);
	}
	free(list);
}
