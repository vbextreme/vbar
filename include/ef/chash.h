#ifndef __EF_CHASH_H__
#define __EF_CHASH_H__

#include <ef/type.h>

#ifdef CHASH_LEN_SIZET
	typedef size_t strlen_t;
#else
	typedef uint16_t strlen_t;
#endif

#ifndef CHASH_MAX_SCAN  
	#define CHASH_MAX_SCAN 30
#endif

#define CHASH_NAME_MEM_FREE 0x01

typedef void(*chashfree_f)(uint32_t hash, const char* name, void* a);
typedef uint32_t(*chash_f)(const char* name, size_t len);
typedef int(*chashcmp_f)(void* a, uint32_t hash, void* data, const char* b);

typedef struct chashElement{
	const char* name;
	void* data;
	strlen_t len;
	uint16_t distance;
	uint16_t flags; 
	uint32_t hash;
}chashElement_s;

typedef struct chash{
	chashElement_s* table;
	size_t size;
	size_t count;
	size_t min;
	size_t maxdistance;
	chashfree_f del;
	chash_f hashing;
}chash_s;

/* hashalg.c */
void hash_seed(uint32_t val);
uint32_t hash_one_at_a_time(const char *key, size_t len);
uint32_t hash_fasthash(const char* data, size_t len);
uint32_t hash_kr(const char*s, size_t len);
uint32_t hash_sedgwicks(const char* str, size_t len);
uint32_t hash_sobel(const char* str, size_t len);
uint32_t hash_weinberger(const char* str, size_t len);
uint32_t hash_elf(const char* str, size_t len);
uint32_t hash_sdbm(const char* str, size_t len);
uint32_t hash_bernstein(const char* str, size_t len);
uint32_t hash_knuth(const char* str, size_t len);
uint32_t hash_partow(const char* str, size_t len);

/*rbhash.c*/
void chash_init(chash_s* ht, size_t max, size_t min, chash_f hashing, chashfree_f del);
void chash_add_fromhash(chash_s* ht, uint32_t hash, const char* name, size_t len, uint16_t flags, void* data);
void chash_add(chash_s* ht, const char* name, size_t len, uint16_t flags, void* data);
void chash_add_dupkey(chash_s* ht, const char* name, size_t len, void* data);
err_t chash_add_unique(chash_s* ht, const char* name, size_t len, uint16_t flags, void* data);
err_t chash_add_unique_dupkey(chash_s* ht, const char* name, size_t len, void* data);
err_t chash_find_fromhash_custom(void** out, chash_s* ht, uint32_t hash, void* name, chashcmp_f cmp);
err_t chash_find_fromhash(void** out, chash_s* ht, uint32_t hash, const char* name, size_t len);
err_t chash_find(void** out, chash_s* ht, const char* name, size_t len);
chashElement_s* chash_find_raw(chash_s* ht, const char* name, size_t len);
void chash_free(chash_s* ht);
err_t chash_remove_fromhash(chash_s* ht, uint32_t hash, const char* name, size_t len);
err_t chash_remove(chash_s* ht, const char* name, size_t len);
size_t chash_mem_usage(chash_s* ht);
size_t chash_bucket_used(chash_s* ht);
size_t chash_collision(chash_s* ht);
size_t chash_distance_max(chash_s* ht);
err_t chash_integrity(chash_s* ht);

#endif
