#include <ef/type.h>
#include <ef/memory.h>
#include <ef/chash.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <string.h>

#define chash_slot_next(SLOT, SIZE) (((SLOT)+1) & ((SIZE)-1))
#define chash_slot_high(HASH,SIZE)	FAST_MOD_POW_TWO((HASH >> 16)^(HASH & 0xFFFF) , SIZE)
#define chash_slot_low(HASH,SIZE)	FAST_MOD_POW_TWO((HASH) >> 16, SIZE)

__private void chash_upsize(chash_s* ht);

__private inline int str_cmp(const char* a, size_t lena, const char* b, size_t lenb){
	if( lena != lenb ) return -1;
	return memcmp(a,b,lena);
}

void chash_init(chash_s* ht, size_t max, size_t min, chash_f hashing, chashfree_f del){
	ht->size = round_up_power_two(max);
	ht->min = min;
	ht->hashing = hashing;
	ht->del = del;
	ht->count = 0;
	ht->maxdistance = 0;
	ht->table = mem_zero_many(chashElement_s, ht->size);
	iassert(ht->table != NULL);
}

__private size_t chash_slot_best(chashElement_s* table, uint32_t slota , uint32_t slotb, size_t size){
	while(1){
		if( table[slota].name == NULL ) return 0;
		if( table[slotb].name == NULL ) return 1;
		slota = chash_slot_next(slota,size);
		slotb = chash_slot_next(slotb,size);
	}
	return 0;
}

__private void chash_swapdown(chash_s* ht, chashElement_s* table, size_t size, chashElement_s rh){ 
	uint32_t slot[2] = {
		[0] = chash_slot_high(rh.hash, size),
		[1] = chash_slot_low(rh.hash, size)
	};

	uint32_t bucket = slot[chash_slot_best(table, slot[0], slot[1], size)];

	while( table[bucket].name != NULL ){
		if(  rh.distance > table[bucket].distance ){
			chashElement_s tmp = table[bucket];
			table[bucket] = rh;
			rh = tmp;
		}
		bucket = chash_slot_next(bucket, size);
		rh.distance++;
		if( rh.distance > ht->maxdistance ) ht->maxdistance = rh.distance;
	}
	table[bucket] = rh;
	return;
}

void chash_add_fromhash(chash_s* ht, uint32_t hash, const char* name, size_t len, uint16_t flags, void* data){
	iassert(name != NULL);
	chashElement_s el = {
		.name = name,
		.flags = flags,
		.len = len,
		.data = data,
		.distance = 0,
		.hash = hash
	};

	chash_swapdown(ht, ht->table, ht->size, el);
	++ht->count;
	if( ht->min ) chash_upsize(ht);
}

void chash_add(chash_s* ht, const char* name, size_t len, uint16_t flags, void* data){
	chash_add_fromhash(ht, ht->hashing(name, len), name, len, flags, data);
}

void chash_add_dupkey(chash_s* ht, const char* name, size_t len, void* data){
	char* dk = mem_many(char, len+1);
	memcpy(dk, name, len+1);
	chash_add(ht, dk, len, CHASH_NAME_MEM_FREE, data);
}

err_t chash_add_unique(chash_s* ht, const char* name, size_t len, uint16_t flags, void* data){
	if( 0 == chash_find(NULL, ht, name, len) ) return -1;
	chash_add(ht, name, len, flags, data);
	return 0;
}

err_t chash_add_unique_dupkey(chash_s* ht, const char* name, size_t len, void* data){
	if( 0 == chash_find(NULL, ht, name, len) ) return -1;
	chash_add_dupkey(ht, name, len, data);
	return 0;
}

err_t chash_find_fromhash_custom(void** out, chash_s* ht, uint32_t hash, void* name, chashcmp_f cmp){
	uint32_t hslot = chash_slot_high(hash, ht->size);
	uint32_t lslot = chash_slot_low(hash, ht->size);
	
	size_t maxscan = ht->maxdistance + 1;

	while( maxscan-->0 ){
		if( ht->table[hslot].name && ht->table[hslot].hash == hash && !cmp(name, hash, ht->table[hslot].data, ht->table[hslot].name) ){
			//dbg_info("find hash:%u hslot:%u name:%s try:%lu", ht->table[hslot].hash, hslot, ht->table[hslot].name, ht->maxdistance +1 - maxscan); 
			if( out ) *out = ht->table[hslot].data;
			return 0;
		}

		if( ht->table[lslot].name && ht->table[lslot].hash == hash && !cmp(name, hash, ht->table[lslot].data, ht->table[lslot].name) ){
			//dbg_info("find hash:%u lslot:%u name:%s try:%lu", ht->table[hslot].hash, lslot, ht->table[hslot].name, ht->maxdistance +1 - maxscan); 
			if( out ) *out = ht->table[lslot].data;
			return 0;
		}
		hslot = chash_slot_next(hslot, ht->size);
		lslot = chash_slot_next(lslot, ht->size);
	}
	return -1;
}

__private long chash_find_bucket(chash_s* ht, uint32_t hash, const char* name, size_t len){
	uint32_t hslot = chash_slot_high(hash, ht->size);
	uint32_t lslot = chash_slot_low(hash, ht->size);
	
	size_t maxscan = ht->maxdistance + 1;

	while( maxscan-->0 ){
		if( ht->table[hslot].name && ht->table[hslot].hash == hash && !str_cmp(name, len, ht->table[hslot].name, ht->table[hslot].len) ){
			//dbg_info("find hash:%u hslot:%u name:%s", ht->table[hslot].hash, hslot, ht->table[hslot].name); 
			return hslot;
		}

		if( ht->table[lslot].name && ht->table[lslot].hash == hash && !str_cmp(name, len, ht->table[lslot].name, ht->table[lslot].len) ){
			//dbg_info("find hash:%u hslot:%u name:%s", ht->table[hslot].hash, hslot, ht->table[hslot].name); 
			return lslot;
		}
		hslot = chash_slot_next(hslot, ht->size);
		lslot = chash_slot_next(lslot, ht->size);
	}
	return -1;
}

err_t chash_find_fromhash(void** out, chash_s* ht, uint32_t hash, const char* name, size_t len){
	long bucket;
	if( (bucket = chash_find_bucket(ht, hash, name, len)) < 0 ){
		return -1;
	}

	if( out ) *out = ht->table[bucket].data;
	return 0;
}

err_t chash_find(void** out, chash_s* ht, const char* name, size_t len){
	uint32_t hash = ht->hashing(name, len);
	return chash_find_fromhash(out, ht, hash, name, len);
}

chashElement_s* chash_find_raw(chash_s* ht, const char* name, size_t len){
	uint32_t hash = ht->hashing(name, len);
	long bucket;
	if( (bucket = chash_find_bucket(ht, hash, name, len)) < 0 ){
		return NULL;
	}
	return &ht->table[bucket];
}

void chash_free(chash_s* ht){
	for( size_t i = 0; i < ht->size; ++i ){
		if( NULL == ht->table[i].name ) continue;
		if(ht->del && ht->table[i].data){
			ht->del(ht->table[i].hash, ht->table[i].name, ht->table[i].data);
		}
		if( ht->table[i].flags & CHASH_NAME_MEM_FREE ){
			free((void*)ht->table[i].name);
		}
		--ht->count;
	}
	free(ht->table);
	iassert(ht->count == 0);
}

__private void chash_swapup(chashElement_s* table, size_t size, size_t bucket){
	size_t bucketfit = bucket;
	bucket = chash_slot_next(bucket, size);

	while( table[bucket].name && table[bucket].distance ){
		table[bucketfit] = table[bucket];
		--table[bucketfit].distance;
		table[bucket].name = 0;
		table[bucket].data = 0;
		bucketfit = bucket;
		bucket = chash_slot_next(bucket, size);
	}
	return;
}

err_t chash_remove_fromhash(chash_s* ht, uint32_t hash, const char* name, size_t len){
	iassert(name != NULL);
	long bucket = chash_find_bucket(ht, hash, name, len);
	if( bucket < 0 ){
		return -1;
	}
	if(ht->del && ht->table[bucket].data){
		ht->del(ht->table[bucket].hash, ht->table[bucket].name, ht->table[bucket].data);
	}
	if( ht->table[bucket].flags & CHASH_NAME_MEM_FREE ){
		free((void*)ht->table[bucket].name);
	}
	ht->table[bucket].name = 0;
	ht->table[bucket].data = 0;
	chash_swapup(ht->table, ht->size, bucket);
	--ht->count;
	return 0;	
}

err_t chash_remove(chash_s* ht, const char* name, size_t len){
	return chash_remove_fromhash(ht, ht->hashing(name, len), name, len);
}

__private void chash_upsize(chash_s* ht){
	size_t p = ((ht->size * ht->min) / 100);
	size_t pm = ht->count + p;
	if( ht->size >= pm ) return;
	size_t newsize = round_up_power_two(ht->count + p);
	chashElement_s* newtable =  mem_zero_many(chashElement_s, newsize);
	iassert( newtable != NULL );

	ht->maxdistance = 0;
	for(size_t i = 0; i < ht->size; ++i){
		if( NULL == ht->table[i].name ) continue;
		ht->table[i].distance = 0;
		chash_swapdown(ht, newtable, newsize, ht->table[i]);
	}
	free(ht->table);
	ht->table = newtable;
	ht->size = newsize;
}

size_t chash_mem_usage(chash_s* ht){
	size_t ram = sizeof(chash_s);
	ram += sizeof(chashElement_s) * ht->size;
	return ram;
}

size_t chash_bucket_used(chash_s* ht){
	return ht->count;
}

size_t chash_collision(chash_s* ht){
	size_t collision = 0;
	for(size_t i =0; i < ht->size; ++i){
		if( ht->table[i].name != NULL && ht->table[i].distance != 0 ) ++collision;
	}
	return collision;
}

size_t chash_distance_max(chash_s* ht){
	size_t distance = 0;
	for(size_t i =0; i < ht->size; ++i){
		if( ht->table[i].name && ht->table[i].distance > distance ){
			distance = ht->table[i].distance;
		}
	}
	return distance;
}

err_t chash_integrity(chash_s* ht){
	err_t ret = 0;
	for(size_t i = 0; i < ht->size; ++i){
		if( ht->table[i].name == NULL) continue;
		uint32_t bucket = ht->table[i].hash;

		uint32_t slot[2] = { 
			[0] = chash_slot_high(bucket, ht->size), 
			[1] = chash_slot_low(bucket, ht->size)
		};

		uint32_t calc = FAST_MOD_POW_TWO(i - ht->table[i].distance, ht->size);
		if( calc != slot[0] && calc != slot[1] ){
			dbg_error("hash:%10u cc:%4u hh:%4u ll:%4u i:%4lu d:%2u s:%s", 
				bucket,
			    calc,	
				slot[0],
				slot[1],
				i,
				ht->table[i].distance,
				ht->table[i].name
			);
			ret = -1;
		}
		else{
			/*dbg_info("hash:%10u cc:%4u hh:%5u ll:%5u i:%4lu d:%2u s:%s", 
				bucket,
				calc,	
				slot[0],
				slot[1],
				i,
				ht->table[i].distance,
				ht->table[i].name
			);*/
		}
	}
	return ret;
}


