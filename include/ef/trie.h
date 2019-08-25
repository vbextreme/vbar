#ifndef __EF_TRIE_H__
#define __EF_TRIE_H__

#include <ef/type.h>

typedef void(*trieFree_f)(void*);

typedef struct trieElement{
	char* charset;
	struct trieElement* next;
	void** endnode;
}trieElement_s;

typedef struct trie{
	trieElement_s root;
	trieFree_f free;
}trie_s;

void trie_init(trie_s* tr, trieFree_f freefnc);
void trie_insert(trie_s* trie, const char* str, void* data);
int trie_search_ch(void** out, trieElement_s** el, char ch);
void* trie_search(trie_s* trie, const char* str);
void trie_free(trie_s* trie);


#endif
