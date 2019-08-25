#ifndef __EF_LIST_H__
#define __EF_LIST_H__

#include <ef/type.h>

typedef struct listDouble{
	struct listDouble* next;
	struct listDouble* prev;
}listDouble_s;

typedef struct listIterator{
	void* begin;
	void* current;
}listIterator_s;


/* double circular linked list with sentinel */ 
void list_dcs_root_init(void* node);
void list_dcs_add_after(void* head, void* node);
void list_dcs_add_before(void* head, void* node);
void* list_dcs_remove(void* node);
int list_dcs_is_empty(void* head);
void list_dcs_iterator_init(listIterator_s* it, void* node);
void* list_dcs_iterator_next(listIterator_s* it);
void* list_dcs_iterator_prev(listIterator_s* it);


#endif
