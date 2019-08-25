#include <ef/vectorization.h>

void __vectorize_begin(valign_s* va, size_t const vsize, size_t const ssize){
	size_t const len = va->end - va->start;
	size_t const asize = vsize/ssize;

	char* mem = va->scalarpre;
	va->scalarpre = &mem[(va->start*ssize)];
	if( vsize == 0 || len < asize ) 
		goto UNALIGNED;
	
	size_t outpre = (size_t)&mem[va->start*ssize] % vsize;
	va->pre = outpre ? (vsize-outpre)/asize : 0 ;
	if( va->pre >= len - asize ) goto UNALIGNED;

	size_t const outali = len - va->pre;
	va->aligned = outali % asize ? (outali - (outpre % asize))/asize : outali/asize;
	va->vector = &mem[(va->start+va->pre)*ssize];

	va->post = len - (va->pre+va->aligned*asize);
	va->scalarpost = &mem[(va->start+va->pre+va->aligned*asize)*ssize];
	
	return;
UNALIGNED:
	va->pre = len;
	va->post = 0;
	va->aligned = 0;
}

void __vectorize_pair_begin(valign_s* va, valign_s* vb, size_t const vsize, size_t const ssize){
	size_t const lena = va->end - va->start;
	size_t const lenb = vb->end - vb->start;
	size_t const asize = vsize/ssize;

	char* mema = va->scalarpre;
	va->scalarpre = &mema[(va->start*ssize)];
	char* memb = vb->scalarpre;
	vb->scalarpre = &memb[(vb->start*ssize)];

	if( vsize == 0 || lena < asize || lenb < asize ) 
		goto UNALIGNED;
	
	size_t outpre = (size_t)&mema[va->start*ssize] % vsize;
	va->pre = outpre ? (vsize-outpre)/asize : 0 ;
	outpre = (size_t)&memb[vb->start*ssize] % vsize;
	vb->pre = outpre ? (vsize-outpre)/asize : 0 ;
	if( va->pre != vb->pre || va->pre >= lena - asize || vb->pre >= lenb - asize ) goto UNALIGNED;

	size_t outali = lena - va->pre;
	va->aligned = outali % asize ? (outali - (outpre % asize))/asize : outali/asize;
	outali = lenb - vb->pre;
	vb->aligned = outali % asize ? (outali - (outpre % asize))/asize : outali/asize;
	if( va->aligned != vb->aligned ) goto UNALIGNED;

	va->post = lena - (va->pre+va->aligned*asize);
	vb->post = lenb - (vb->pre+vb->aligned*asize);

	va->scalarpost = &mema[(va->start+va->aligned*asize)*ssize];
	vb->scalarpost = &memb[(vb->start+vb->aligned*asize)*ssize];

	va->vector = &mema[(va->start+va->pre)*ssize];
	vb->vector = &memb[(vb->start+vb->pre)*ssize];

	return;
UNALIGNED:
	va->pre = lena;
	vb->pre = lenb;
	va->post = 0;
	vb->post = 0;
	va->aligned = 0;
	vb->aligned = 0;
}


