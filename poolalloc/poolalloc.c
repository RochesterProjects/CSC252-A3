#include "dbll.h"
#include <stdlib.h>
#include "poolalloc.h"

/*
   a pool-based allocator that uses doubly-linked lists to track
   allocated and free blocks
 */

/* create and initialize a memory pool of the required size */
/* use malloc() or calloc() to obtain this initial pool of memory from the system */
struct memory_pool *mpool_create(size_t size)
{

  /* set start to memory obtained from malloc */
  /* set size to size */

  /* create a doubly-linked list to track allocations */
  /* create a doubly-linked list to track free blocks */

  /* create a free block of memory for the entire pool and place it on the free_list */

  /* return memory pool object */

  return NULL;
}

/* ``destroy'' the memory pool by freeing it and all associated data structures */
/* this includes the alloc_list and the free_list as well */
void mpool_destroy(struct memory_pool *p)
{
  /* make sure the allocated list is empty (i.e. everything has been freed) */
  /* free the alloc_list dbll */
  /* free the free_list dbll  */
  /* free the memory pool structure */
}


/* allocate a chunk of memory out of the free pool */

/* Return NULL if there is not enough memory in the free pool */

/* The address you return must be aligned to 1 (for size=1), 2 (for
   size=2), 4 (for size=3,4), 8 (for size=5,6,7,8). For all other
   sizes, align to 16.
*/

void *mpool_alloc(struct memory_pool *p, size_t size)
{

  /* check if there is enough memory for allocation of `size` (taking
	 alignment into account) by checking the list of free blocks */

  /* search the free list for a suitable block */
  /* there are many strategies you can use: first fit (the first block that fits),
	 best fit (the smallest block that fits), etc. */

  /* if no suitable block can be found, return NULL */

  /* if found, create an alloc_info node, store start of new region
	 into offset, set size to allocation size (take alignment into
	 account!), set free to null */

  /* add the new alloc_info node to the memory pool's allocated
	 list */

  /* return pointer to allocated region*/

  return NULL;

}

/* Free a chunk of memory out of the pool */
/* This moves the chunk of memory to the free list. */
/* You may want to coalesce free blocks [i.e. combine two free blocks
   that are are next to each other in the pool into one larger free
   block. Note this requires that you keep the list of free blocks in order */
void mpool_free(struct memory_pool *p, void *addr)
{
  /* search the alloc_list for the block */
  /* move it to the free_list */
  /* coalesce the free_list */
}
