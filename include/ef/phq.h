#ifndef __EF_PHQ_H__
#define __EF_PHQ_H__

#include <ef/type.h>

typedef int (*phqCompare_f)(size_t a, size_t b);
typedef void (*phqFree_f)(void* data);

typedef struct phqElement{
	size_t priority;
	size_t index;
	phqFree_f free;
	void* data;
}phqElement_s;

typedef struct phq{
	size_t size;
	size_t count;
	size_t resize;
	phqCompare_f cmp;
	phqElement_s** elements;
}phq_s;

int phq_cmp_des(size_t a, size_t b);
int phq_cmp_asc(size_t a, size_t b);
err_t phq_init(phq_s* q, size_t size, size_t resize, phqCompare_f cmp);
void phq_element_free(phqElement_s *el);
void phq_free(phq_s *q);
size_t phq_size(phq_s *q);
size_t phq_count(phq_s *q);
phqElement_s* phq_element_new(size_t priority, void* data, phqFree_f pfree);
err_t phq_insert(phq_s *q, phqElement_s* el);
void phq_change_priority(phq_s *q, size_t newpri, phqElement_s* el);
void phq_remove(phq_s *q, phqElement_s* el);
phqElement_s* phq_pop(phq_s *q);
phqElement_s* phq_peek(phq_s *q);
void phq_dump(phq_s *q);

#endif
