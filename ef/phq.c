/* fork from Volkan Yazıcı priority heap queue */

#include <ef/type.h>
#include <ef/memory.h>
#include <ef/phq.h>

#define left(i)   ((i) << 1)
#define right(i)  (((i) << 1) + 1)
#define parent(i) ((i) >> 1)

int phq_cmp_des(size_t a, size_t b){
	return (a < b);
}

int phq_cmp_asc(size_t a, size_t b){
	return (a > b);
}

err_t phq_init(phq_s* q, size_t size, size_t resize, phqCompare_f cmp){
	++size;
	
	q->elements = mem_many(phqElement_s*, size);
	iassert(q->elements != NULL);
	memset(q->elements,0,sizeof(phqElement_s*) * size);

    q->size = size;
    q->count = 0;
	q->resize = resize;
	q->cmp = cmp;
    return 0;
}

void phq_element_free(phqElement_s *el){
	if( el->free ) el->free(el->data);
	free(el);
}

void phq_free(phq_s *q){
	for( size_t i = 0; i < q->size; ++i ){
		if( q->elements[i] ){
			phq_element_free(q->elements[i]);
		}
	}
    free(q->elements);
}

size_t phq_size(phq_s *q){
    return (q->size - 1);
}

size_t phq_count(phq_s *q){
    return (q->count);
}

__private void bubble_up(phq_s *q, size_t i){
    size_t parentnode;
    phqElement_s* movingnode = q->elements[i];
    size_t movingpri = movingnode->priority;

    for( parentnode = parent(i); ((i > 1) && q->cmp(q->elements[parentnode]->priority, movingpri)); i = parentnode, parentnode = parent(i)){
        q->elements[i] = q->elements[parentnode];
        q->elements[i]->index = i;
    }

    q->elements[i] = movingnode;
	q->elements[i]->index = i;
}

__private size_t maxchild(phq_s *q, size_t i){
    size_t childnode = left(i);

    if (childnode > q->count) return 0;
    if ( (childnode+1) < q->count && q->cmp(q->elements[childnode]->priority, q->elements[childnode+1]->priority) ) childnode++;

    return childnode;
}

__private void percolate_down(phq_s *q, size_t i){
    size_t childnode;
    phqElement_s* movingnode = q->elements[i];
    size_t movingpri = movingnode->priority;

    while( (childnode = maxchild(q, i)) && q->cmp(movingpri, q->elements[childnode]->priority) ){
        q->elements[i] = q->elements[childnode];
        q->elements[i]->index = i;
        i = childnode;
    }

    q->elements[i] = movingnode;
    movingnode->index = i;
}

phqElement_s* phq_element_new(size_t priority, void* data, phqFree_f pfree){
	phqElement_s* el = mem_new(phqElement_s);
	iassert(el != NULL);
	el->priority = priority;
	el->index = 0;
	el->data = data;
	el->free = pfree;
	return el;
}

err_t phq_insert(phq_s *q, phqElement_s* el){
    iassert( q != NULL );
	iassert( el != NULL );

    if( q->count >= q->size -1 ){
        size_t newsize = q->size + q->resize;
		phqElement_s** tmp = realloc(q->elements, sizeof(phqElement_s*) * newsize);
        if( tmp == NULL ) return -1;
        q->elements = tmp;
        q->size = newsize;
    }

    size_t i = ++q->count;
    q->elements[i] = el;
    bubble_up(q, i);
    return 0;
}

void phq_change_priority(phq_s *q, size_t newpri, phqElement_s* el){
    size_t posn = el->index;
    size_t oldpri = el->priority;
	el->priority = newpri;
    
    if( q->cmp(oldpri, newpri) )
        bubble_up(q, posn);
    else
        percolate_down(q, posn);
}

void phq_remove(phq_s *q, phqElement_s* el){
    size_t posn = el->index;
    q->elements[posn] = q->elements[--q->size];
    if( q->cmp(el->priority, q->elements[posn]->priority) )
        bubble_up(q, posn);
    else
        percolate_down(q, posn);
}

phqElement_s* phq_pop(phq_s *q){	
	iassert( q );
	if( q->count == 0 ) return NULL;

    phqElement_s* head = q->elements[1];
    q->elements[1] = q->elements[q->count--];
	percolate_down(q, 1);
    return head;
}

phqElement_s* phq_peek(phq_s *q){
    iassert( q != NULL );
	if( q->count == 0) return NULL;
    return q->elements[1];
}

void phq_dump(phq_s *q){
    for(size_t i = 1; i < q->count; ++i){
        dbg_info("[%lu-%lu] priority=%lu data=%p", i, q->elements[i]->index, q->elements[i]->priority, q->elements[i]->data); 
    }
}


