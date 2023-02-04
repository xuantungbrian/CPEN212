#include <stdlib.h>
#include <string.h>
#include "cpen212alloc.h"
#include <stdio.h>
#include <inttypes.h>
#define BitVal(data,y) ( (data>>y) & 1)

typedef struct {
    void *end;   // end of heap
    void *free;  // next free address on heap
} alloc_state_t;

void *cpen212_init(void *heap_start, void *heap_end) {
	if (heap_start>=heap_end) return NULL;
	if (heap_end-heap_start<16) return NULL;
	uint64_t mask = 9223372036854775807ULL;
    	uint64_t mask3= 9223372036854775808ULL;
    	alloc_state_t *s = (alloc_state_t *) malloc(sizeof(alloc_state_t));
    	s->end = heap_end;
    	s->free = heap_start;
    	*(uint64_t *)heap_start = (*(uint64_t *)heap_start&mask3)|(heap_end - heap_start);
	*(uint64_t *)heap_start = *(uint64_t *)heap_start&mask;
	*(uint64_t *)(heap_end - 8) = *(uint64_t *)(heap_end - 8)&mask;
	*(uint64_t *)(heap_end - 8) = (*(uint64_t *)(heap_end - 8)&mask3)|(heap_end - heap_start);
    	return s;
}

void cpen212_deinit(void *s) {	
    free(s);
}

/*Structure of the allocated block:
 *The allocated block starts with a 8 bytes header. First 63 bits of the header will be used to store 
 *the size of the block. The last bit is used as a flag to check whether the block is free or not.
 *Nbytes requested will be made to the nearest but bigger number which is a multiple of 8. Call this 
 *number aligned_sz. After the the header, there is a space of aligned_sz bytes to use as memmory. 
 *After this space is the footer, which is designed similar to the header (8 bytes and used to store 
 *size of the block at first 63 bits, last bit is flag). So a block contains 8 bytes header, a space 
 *used as memmory and 8 bytes footer. Size of the block will be 8+aligned_sz+8.
 * */
void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    size_t aligned_sz = (nbytes + 7) & ~7;
    size_t original_sz = 0;
    void *p = s->free;
    uint64_t a;
    uint64_t mask = 9223372036854775807ULL;
    uint64_t mask3= 9223372036854775808ULL;
    while (1) {
		if (p + 16 + aligned_sz > s->end || nbytes==0) return NULL;
		a=(uint64_t)BitVal(*(uint64_t *)p, 63);
		if ((a == 0) && (((uint64_t)(*(uint64_t *)p&mask - 16)) == (uint64_t)aligned_sz)) {
			*(uint64_t *)p = (*(uint64_t *)p&mask3)|(aligned_sz + 16);
                        *(uint64_t *)p = *(uint64_t *)p|mask3;
                        *(uint64_t *)(p + 8 + aligned_sz) = (*(uint64_t *)(p + 8 + aligned_sz)&mask3)|(aligned_sz + 16);
                        *(uint64_t *)(p + 8 + aligned_sz) = *(uint64_t *)(p + 8 + aligned_sz)|mask3;
			return p + 8;
		}
		else if ( (a == 0) && (((uint64_t)(*(uint64_t *)p&mask - 16)) > (uint64_t)aligned_sz)) {
			original_sz = *(uint64_t *)p;
			*(uint64_t *)p = (*(uint64_t *)p&mask3)|(aligned_sz + 16);
			*(uint64_t *)p = *(uint64_t *)p|mask3;
			*(uint64_t *)(p + 8 + aligned_sz) = (*(uint64_t *)(p + 8 + aligned_sz)&mask3)|(aligned_sz + 16);
			*(uint64_t *)(p + 8 + aligned_sz) = *(uint64_t *)(p + 8 + aligned_sz)|mask3;

			*(uint64_t *)(p + 16 + aligned_sz) = (*(uint64_t *)(p + 16 + aligned_sz)& mask3)|(original_sz - aligned_sz - 16);
			*(uint64_t *)(p + 16 + aligned_sz) = *(uint64_t *)(p + 16 + aligned_sz)&mask;
			*(uint64_t *)(p + original_sz - 8) = (*(uint64_t *)(p + original_sz - 8) & mask3) | (original_sz - aligned_sz - 16  << 0);
			*(uint64_t *)(p + original_sz - 8) = *(uint64_t *)(p + original_sz - 8)&mask;
			return p + 8;
		}
		else {
			p = p + (*(uint64_t *)p&mask);
		}
	}
}

void cpen212_free(void *s, void *p) {
	uint64_t mask=9223372036854775807ULL;
	*(uint64_t *)(p - 8) = *(uint64_t *)(p - 8)&mask;
	uint64_t sz = *(uint64_t *)(p - 8)&mask;
	*(uint64_t *)(p-8 + sz - 8) = *(uint64_t *)(p-8 + sz - 8)&mask;
}

/*// description:
// - extend or shrink allocated block size, moving the block address and contents,deallocating the previous block if necessary
// arguments:
// - alloc_state: the pointer returned by your cpen212_init()
// - prev: a pointer previously returned from cpen212_alloc() and never freed,
//         or NULL (in which case realloc behaves like alloc with the same size)
// returns:
// - pointer p to the allocated block of at least nbytes bytes,
//   with a copy of the previous block at prev if prev is not NULL
//   (p may be NULL if nbytes was 0 or if heap is full)
// output invariants:
// - p must be either in [heap_start,heap_end) as passed to cpen212_init, or NULL
// - if p is not NULL, P+nbytes must be in (heap_start,heap_end] as passed to cpen212_init
// - p must be aligned on an 8-byte boundary
// - [p,p+nbytes) may be overwritten by the caller
// - if prev is not NULL, the full contents of the block at prev
//   must be copied to the beginning of the block at p
//   (or nbytes of the previous contents if shrinking)
 *
 *
 * */
void *cpen212_realloc(void *s, void *prev, size_t nbytes) {
    void *p = cpen212_alloc(s, nbytes);
    uint64_t mask=9223372036854775807ULL;
    uint64_t aligned_sz = (nbytes + 7) & ~7;
    uint64_t original_sz;
    if (p != NULL && prev != NULL) {
	original_sz = *(uint64_t *)(prev - 8)&mask-16;
	if (original_sz >= (uint64_t) aligned_sz) {
		memmove(p, prev, aligned_sz); // see WARNING below
	}
	else {
		memmove(p, prev, original_sz);
	}
	cpen212_free(s, prev);
	}
    return p;
}

// WARNING: we don't know the prev block's size, so memmove just copies nbytes here.
//          this is safe only because in this dumb allocator we know that prev < p,
//          and p has at least nbytes usable. in your implementation,
//          you probably need to use the original allocation size.

bool cpen212_check_consistency(void *alloc_state) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    return s->end > s->free;
}

