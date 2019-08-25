#include <ef/type.h>
#include <stdlib.h>
#include <string.h>

__private uint32_t hashseed;

void hash_seed(uint32_t val){
	hashseed = val;
}

/*********************/
/*** ONE AT A TIME ***/
/*********************/

uint32_t hash_one_at_a_time(const char *key, size_t len){
	uint32_t hash, i;
	for (hash = 0, i = 0; i < len; ++i){
	    hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
} 

/*****************/
/*** FAST HASH ***/
/*****************/

//#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
    #define g16b(d) (*((const uint16_t *) (d)))
//#endif

//#if !defined (g16b)
  //  #define g16b(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) + (uint32_t)(((const uint8_t *)(d))[0]) )
//#endif

uint32_t hash_fasthash(const char* data, size_t len){
    uint32_t hash = len;
    uint32_t tmp;
	uint32_t partial;
    int rem;

    //if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    for (;len > 0; len--){
        hash  += g16b(data);
//        tmp    = (g16b(data+2) << 11) ^ hash;
        partial= (g16b(data+2) << 11);
	    tmp = partial ^ hash;

        hash   = (hash << 16) ^ tmp;
        data  += 2 * sizeof(int16_t);
        hash  += hash >> 11;
    }

    switch (rem) {
        case 3: 
			hash += g16b (data);
            hash ^= hash << 16;
            hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
            hash += hash >> 11;
        break;
        
		case 2: 
			hash += g16b(data);
            hash ^= hash << 11;
            hash += hash >> 17;
        break;
        
		case 1: 
			hash += (signed char)*data;
            hash ^= hash << 10;
            hash += hash >> 1;
		break;
    }

    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
	return hash;
}

/***********/
/*** K&R ***/
/***********/

uint32_t hash_kr(const char*s, __unused size_t len){
	size_t hash;
	for( hash = 0; *s; ++s)
		hash = *s + 31 * hash;
	return hash;
}

/************************/
/*** Robert Sedgwicks ***/
/************************/

uint32_t hash_sedgwicks(const char* str, size_t len){
   uint32_t b = 378551;
   uint32_t a = 63689;
   uint32_t hash = 0;
   uint32_t i = 0;

   for (i = 0; i < len; ++i){
      hash = hash * a + str[i];
      a = a * b;
   }

   return hash;
}

/********************/
/*** Justin Sobel ***/
/********************/

uint32_t hash_sobel(const char* str, size_t len){
   uint32_t hash = 1315423911;
   uint32_t i = 0;

   for (i = 0; i < len; ++i){
      hash ^= ((hash << 5) + str[i] + (hash >> 2));
   }

   return hash;
}

/***************************/
/*** Peter J. Weinberger ***/
/***************************/

uint32_t hash_weinberger(const char* str, size_t len){
   const uint32_t BitsInUnsignedInt = (uint32_t)(sizeof(uint32_t) * 8);
   const uint32_t ThreeQuarters     = (uint32_t)((BitsInUnsignedInt  * 3) / 4);
   const uint32_t OneEighth         = (uint32_t)(BitsInUnsignedInt / 8);
   const uint32_t HighBits          = (uint32_t)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
   uint32_t hash = 0;
   uint32_t test = 0;
   uint32_t i    = 0;

   for (i = 0; i < len; ++i){
      hash = (hash << OneEighth) + str[i];
      if ((test = hash & HighBits) != 0){
         hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
      }
   }

   return hash;
}

/************/
/*** UNIX ***/
/************/

uint32_t hash_elf(const char* str, size_t len){
   uint32_t hash = 0;
   uint32_t x    = 0;
   uint32_t i    = 0;

   for (i = 0; i < len; ++i){
      hash = (hash << 4) + str[i];
      if ((x = hash & 0xF0000000L) != 0){
         hash ^= (x >> 24);
      }
      hash &= ~x;
   }

   return hash;
}

/************/
/*** SDBM ***/
/************/

uint32_t hash_sdbm(const char* str, size_t len){
   uint32_t hash = 0;
   uint32_t i    = 0;

   for (i = 0; i < len; ++i){
      hash = str[i] + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}

/***************************/
/*** Daniel J. Bernstein ***/
/***************************/

uint32_t hash_bernstein(const char* str, size_t len){
   uint32_t hash = 5381;
   uint32_t i    = 0;

   for (i = 0; i < len; ++i){
      hash = ((hash << 5) + hash) + str[i];
   }

   return hash;
}

/************************/
/*** Donald E. Knuth ***/
/************************/

uint32_t hash_knuth(const char* str, size_t len){
   uint32_t hash = len;
   uint32_t i    = 0;

   for (i = 0; i < len; ++i){
      hash = ((hash << 5) ^ (hash >> 27)) ^ str[i];
   }

   return hash;
}

/********************/
/*** Arash Partow ***/
/********************/

uint32_t hash_partow(const char* str, size_t len)
{
   uint32_t hash = 0xAAAAAAAA;
   uint32_t i    = 0;

   for (i = 0; i < len; ++i){
      hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ str[i] * (hash >> 3)) :
                               (~((hash << 11) + (str[i] ^ (hash >> 5))));
   }

   return hash;
}

