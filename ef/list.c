#include <ef/list.h>

/*********************************************/
/* double circular linked list with sentinel */
/*********************************************/

void list_dcs_root_init(void* node){
	listDouble_s* no = node;
	no->next = no->prev = no;
}

void list_dcs_add_after(void* head, void* node){
	listDouble_s* he = head;
	listDouble_s* no = node;
	no->next = he->next;
	he->next->prev = no;
	he->next = no;
	no->prev = he;
}

void list_dcs_add_before(void* head, void* node){
	listDouble_s* he = head;
	listDouble_s* no = node;
	no->prev = he->prev;
	he->prev->next = no;
	he->prev = no;
	no->next = he;
}

void* list_dcs_remove(void* node){
	listDouble_s* no = node;
	no->prev->next = no->next;
	no->next->prev = no->prev;
	no->next = NULL;
	no->prev = NULL;
	return no;
}

int list_dcs_is_empty(void* head){
	listDouble_s* no = head;
	return no->next == no ? 1 : 0;
}

void list_dcs_iterator_init(listIterator_s* it, void* node){
	listDouble_s* no = node;
	it->begin = node;
	it->current = no->next;
}

void* list_dcs_iterator_next(listIterator_s* it){
	if( it->begin == it->current ) return NULL;
	listDouble_s* no = it->current;
	it->current = no->next;
	return no;
}

void* list_dcs_iterator_prev(listIterator_s* it){
	if( it->begin == it->current ) return NULL;
	listDouble_s* no = it->current;
	it->current = no->prev;
	return no;
}







