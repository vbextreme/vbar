#include <ef/type.h>
#include <ef/memory.h>
#include <ef/strong.h>
#include <ef/trie.h>

void trie_init(trie_s* tr, trieFree_f freefnc){
	tr->root.charset = NULL;
	tr->root.endnode = NULL;
	tr->root.next = NULL;
	tr->free = freefnc;
}

__private void trie_rec_free(trieFree_f fnc, trieElement_s* el){
	if( el->charset == NULL ) return;
	for( size_t i = 0; el->charset[i] ; ++i){
		if( fnc && el->endnode[i] ) fnc(el->endnode[i]);
		trie_rec_free(fnc, &el->next[i]);
	}
	free(el->charset);
	free(el->endnode);
	free(el->next);
}

void trie_free(trie_s* tr){
	trie_rec_free(tr->free, &tr->root);
}

__private inline int trie_search_charset(char* cset, char ch){
	if( cset == NULL ) return -1;
	char* set = strchr(cset, ch);
	if( set ) return set - cset;
	return -1;
}

__private trieElement_s* trie_add_charset(trieElement_s* node, char ch, void* data){
	size_t size;
	if( node->charset == NULL ){
		size = 0;
		//dbg_info("null mode");
		iassert( node->next == NULL );
		iassert( node->endnode == NULL );
	}
	else{
		size = strlen(node->charset);
		//dbg_info("resize %lu mode", size);
		iassert( node->next != NULL );
		iassert( node->endnode != NULL );
	}
	++size;
	char* rs = realloc(node->charset, size+1 );
	iassert(rs != NULL);
	node->charset = rs;
	rs[size-1] = ch;
	rs[size] = 0;

	void** rsv = realloc(node->endnode, size * sizeof(void*));
	iassert( rsv != NULL );
	node->endnode = rsv;
	rsv[size-1] = data;

	trieElement_s* trv = realloc(node->next, size * sizeof(trieElement_s));
	iassert( trv != NULL );
	node->next = trv;
	trv[size-1].charset = NULL;
	trv[size-1].endnode = NULL;
	trv[size-1].next = NULL;
	return &trv[size-1];
}


#if DEBUG_ENABLE

__private char* term_escape_escape(int ch){
	static char ret[2] = {0};
   	ret[0] = ch;
	switch( ch ){
		case 1: return "\1";
		case 0x1B: return "\\E";
		case '\a': return "\\A";
		case '\b': return "\\B";
		case '\f': return "\\F";
		case '\n': return "\\N";
		case '\r': return "\\R";
		case '\t': return "\\T";
		case '\v': return "\\V";
		default: return ret;
	}
}

#endif





__private trieElement_s* trie_add_ch(trieElement_s* node, char ch, void* data){
	int chs = trie_search_charset(node->charset, ch);
	if( chs < 0 ){
		//dbg_info("new charset '%s'", term_escape_escape(ch));
		return trie_add_charset(node, ch, data);
	}
	if( data ){
		//dbg_info("skip charset '%s'", term_escape_escape(ch));
		if( node->endnode[chs] ) dbg_warning("collision check");
		node->endnode[chs] = data;
	}
	return &node->next[chs];
}

void trie_insert(trie_s* trie, const char* str, void* data){
	if( NULL == str || *str == 0 ) return ;

	trieElement_s* el = &trie->root;
	while( *(str+1) != 0 ){
		el = trie_add_ch(el, *str++, NULL);
	}
	trie_add_ch(el, *str, data);
}

int trie_search_ch(void** out, trieElement_s** el, char ch){
	int chs = trie_search_charset((*el)->charset, ch);
	if( chs < 0 ){
		dbg_warning("search '%s' not exists", term_escape_escape(ch));
		return -1;
	}
	int ret = (*el)->endnode[chs] ? 1 : 0;
	*out=(*el)->endnode[chs];
	*el = &(*el)->next[chs];
	return ret;
}

void* trie_search(trie_s* trie, const char* str){
	trieElement_s* el = &trie->root;
	int fmode;
	void* out;
	while( *str && (fmode=trie_search_ch(&out, &el, *str)) >=0 ){
		++str;
		if( fmode == 1 && *str == 0 ){
			return out;
		}
	}
	return NULL;
}










