#ifndef __EF_VECTORIZATION_H__
#define __EF_VECTORIZATION_H__

#include <ef/type.h>

typedef struct valign{
	size_t pre;
	size_t aligned;
	size_t post;
	size_t start;
	size_t end;
	void* scalarpre;
	void* scalarpost;
	void* vector;
}valign_s;

//#ifdef VECTORIZE

#define __vector4 __attribute__((vector_size(4)))
#define __vector8 __attribute__((vector_size(8)))
#define __vector16 __attribute__((vector_size(16)))
#define __vector32 __attribute__((vector_size(32)))
#define __vector64 __attribute__((vector_size(64)))
#define __vector_aligned(N) __attribute__((aligned(__BIGGEST_ALIGNMENT__)))
#define __is_aligned(EXP,N) __builtin_assume_aligned(EXP,N)
#define __is_aligned_but(EXP,N,UNALIGNED) __builtin_assume_aligned(EXP,N,UNALIGNED)

typedef unsigned char uchar8_v __vector8;
typedef unsigned char uchar16_v __vector16;
typedef unsigned char uchar32_v __vector32;
typedef unsigned int uint4_v __vector16;
typedef unsigned int uint8_v __vector32;
typedef float float4_v __vector16;
typedef float float8_v __vector32;

#define vectorize_loop(VECTYPE, TYPE, PTR, START, END, SCALARBODY, VECTORBODY) do{\
	valign_s __va__ = {\
		.start = START,\
		.end = END,\
		.scalarpre = PTR,\
		.scalarpost = NULL,\
		.vector = NULL\
	};\
	__vectorize_begin(&__va__, sizeof(VECTYPE), sizeof(TYPE));\
	TYPE* scalar = __va__.scalarpre;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.pre; ++__iterator__) SCALARBODY\
	VECTYPE* vector = __is_aligned(__va__.vector,sizeof(VECTYPE));\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.aligned; ++__iterator__) VECTORBODY\
	scalar = __va__.scalarpost;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.post; ++__iterator__) SCALARBODY\
}while(0)

#define vectorize_pair_loop(VECTYPE, TYPE, PTRA, STARTA, ENDA, PTRB, STARTB, ENDB, SCALARBODY, VECTORBODY) do{\
	valign_s __va__ = {\
		.start = STARTA,\
		.end = ENDA,\
		.scalarpre = PTRA,\
		.scalarpost = NULL,\
		.vector = NULL\
	};\
	valign_s __vb__ = {\
		.start = STARTB,\
		.end = ENDB,\
		.scalarpre = PTRB,\
		.scalarpost = NULL,\
		.vector = NULL\
	};\
	__vectorize_pair_begin(&__va__, &__vb__, sizeof(VECTYPE), sizeof(TYPE));\
	TYPE* Ascalar = __va__.scalarpre;\
	TYPE* Bscalar = __vb__.scalarpre;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.pre; ++__iterator__) SCALARBODY\
	VECTYPE* Avector = __is_aligned(__va__.vector,sizeof(VECTYPE));\
	VECTYPE* Bvector = __is_aligned(__vb__.vector,sizeof(VECTYPE));\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.aligned; ++__iterator__) VECTORBODY\
	Ascalar = __va__.scalarpost;\
	Bscalar = __vb__.scalarpost;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.post; ++__iterator__) SCALARBODY\
}while(0)

#define vector4_set_all(VAL) {VAL,VAL,VAL,VAL}
#define vector16_set_all(VAL) {VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL,VAL}

void __vectorize_begin(valign_s* va, size_t const vsize, size_t const ssize);
void __vectorize_pair_begin(valign_s* va, valign_s* vb, size_t const vsize, size_t const ssize);
/*
#else

#define __vector4 
#define __vector8 
#define __vector16 
#define __vector32 
#define __vector64 
#define __vector_aligned(N)
#define __is_aligned(EXP,N) 
#define __is_aligned_but(EXP,N,UNALIGNED)

typedef unsigned char uchar8_v;
typedef unsigned char uchar16_v;
typedef unsigned char uchar32_v;
typedef unsigned int uint4_v;
typedef unsigned int uint8_v;
typedef float float4_v;
typedef float float8_v;

#define vectorize_loop(VECTYPE, TYPE, PTR, START, END, SCALARBODY, VECTORBODY) do{\
	valign_s __va__ = {\
		.start = START,\
		.end = END,\
		.scalarpre = PTR,\
		.pre = END - START,\
		.scalarpost = NULL,\
		.vector = NULL\
	};\
	TYPE* scalar = __va__.scalarpre;\
	for(unsigned __iterator__ = 0; __iterator__ < __va__.pre; ++__iterator__) SCALARBODY\
}while(0)

#define vector4_set_all(VAL) {VAL}
#define vector16_set_all(VAL) {VAL}
#endif
*/
#endif
