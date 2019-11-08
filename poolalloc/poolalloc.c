#include "dbll.h"
#include <stdlib.h>
#include "poolalloc.h"

/*
   a pool-based allocator that uses doubly-linked lists to track
   allocated and free to_adds
 */

/* create and initialize a memory pool of the required size */
/* use malloc() or calloc() to obtain this initial pool of memory from the system */
struct memory_pool *mpool_create(size_t size)
{
  struct memory_pool *mpool = (struct memory_pool *)malloc(sizeof(struct memory_pool ));
  if(mpool == NULL){
    return NULL;
  }
  /* set start to memory obtained from malloc */
  mpool->start = malloc(size);
  /* set size to size */
  mpool->size = size;
  /* create a doubly-linked list to track allocations */
  mpool->alloc_list = dbll_create();

  /* create a doubly-linked list to track free to_adds */
  mpool->free_list = dbll_create();

  /* create a free to_add of memory for the entire pool and place it on the free_list */
  struct alloc_info *mem_to_add = (struct alloc_info*) malloc(sizeof(struct alloc_info));
  mem_to_add->size = size;
  mem_to_add->offset = 0;
  mem_to_add->request_size = 0;
  dbll_append(mpool->free_list, mem_to_add);
  /* return memory pool object */
  return mpool;

}

/* ``destroy'' the memory pool by freeing it and all associated data structures */
/* this includes the alloc_list and the free_list as well */
void mpool_destroy(struct memory_pool *p)
{
  /* make sure the allocated list is empty (i.e. everything has been freed) */
  /* free the alloc_list dbll */
  dbll_free(p->alloc_list);
  /* free the free_list dbll  */
  dbll_free(p->free_list);
  
  /* free the memory pool structure */
  free(p);
}




/* allocate a chunk of memory out of the free pool */

/* Return NULL if there is not enough memory in the free pool */

/* The address you return must be aligned to 1 (for size=1), 2 (for
   size=2), 4 (for size=3,4), 8 (for size=5,6,7,8). For all other
   sizes, align to 16.
*/

void *mpool_alloc(struct memory_pool *p, size_t size)
{

  size_t align;
  switch (size) {
    case 1:
      align = 1;
      break;
    case 2:
      align = 2;
      break;
    case 3:
      align = 4;
    case 4:
      align = 4;
      break;
    case 5:
      align = 8;
      break;
    case 6:
      align = 8;
      break;
    case 7:
      align = 8;
      break;
    case 8:
      align = 8;
      break;
    default:
      align = 16;
      break;
  }
  /* check if there is enough memory for allocation of `size` (taking
   alignment into account) by checking the list of free blocks */

  /* search the free list for a suitable block */
  /* there are many strategies you can use: first fit (the first block that fits),
   best fit (the smallest block that fits), etc. */
  struct llnode* block = NULL;
  struct llnode* curr = p->free_list->first;
  while (curr != NULL) {
    struct alloc_info* temp = curr->user_data;
    size_t start = temp->offset;
    if (start % align != 0){
      start = (start / align + 1) * align;
    }
    if (temp->offset + temp->size >= start + size) {
      block = curr;
      break;
    }
  curr = curr->next;
  }

  /* if no suitable block can be found, return NULL */
  if (block == NULL){return NULL;}
  struct alloc_info* block_data = block->user_data;

  /* if found, create an alloc_info block, store start of new region
   into offset, set size to allocation size (take alignment into
   account!), set free to null */
  struct alloc_info* to_add;
  if (block_data->offset % align != 0) {
    // Split memory to_add into 2 to_adds: offset->alignment-1 and alignment->offset+size 
    to_add = malloc( sizeof(struct alloc_info) );
    to_add->size = (block_data->offset / align + 1) * align - block_data->offset;
    to_add->offset = block_data->offset;

    block_data->offset += to_add->size;
    block_data->size -= to_add->size;

    dbll_insert_before(p->free_list, block, to_add);
  }

  to_add = malloc( sizeof(struct alloc_info) );
  to_add->size = size;
  to_add->offset = block_data->offset;
  to_add->request_size = size;

  block_data->offset += to_add->size;
  block_data->size -= to_add->size;
  if (block_data->size == 0){
    dbll_remove(p->free_list, block);
  }

  /* add the new alloc_info block to the memory pool's allocated
   list */
  dbll_append(p->alloc_list, to_add);

  /* return pointer to allocated region*/
  return p->start + to_add->offset;

}

/* Free a chunk of memory out of the pool */
/* This moves the chunk of memory to the free list. */
/* You may want to coalesce free to_adds [i.e. combine two free to_adds
   that are are next to each other in the pool into one larger free
   to_add. Note this requires that you keep the list of free to_adds in order */
void mpool_free(struct memory_pool *p, void *addr)
{
  /* search the alloc_list for the to_add */
  struct llnode* block = NULL;
  struct llnode* curr = p->alloc_list->first;
  while (curr != NULL){ 
    if (((struct alloc_info*) curr->user_data)->offset + p->start == addr) {
      block = curr;
      break;
    }

  if(block == NULL){
    return;
  }
   curr = curr->next;
  }

  /* move it to the free_list */
  struct alloc_info* data = block->user_data;
  // Remove from alloc_list
  dbll_remove(p->alloc_list, block);

  // Add to free_list
  block = NULL;
  curr = p->free_list->first;
  while( curr != NULL){ 
    if (((struct alloc_info*) curr->user_data)->offset > (size_t) addr - (size_t) p->start) {
      block = curr;
      break;
    }
    curr = curr->next;
  }
  
  if (block != NULL){
    block = dbll_insert_before(p->free_list, block, data);
  }
  else {
    block = dbll_append(p->free_list, data);
  }

  /* coalesce the free_list */

  // coalesce the previous block
  if (block->prev != NULL) {
    struct llnode* prev = block->prev;
    struct alloc_info* current_data = block->user_data;
    struct alloc_info* prev_data = prev->user_data;
    if (prev_data->offset + prev_data->size == current_data->offset) {
      current_data->offset -= prev_data->size;
      current_data->size += prev_data->size;
      dbll_remove(p->free_list, prev);
    }
  }

  // coalesce the next block
  if  (block->next != NULL) {
    struct llnode* next = block->next;
    struct alloc_info* current_data = block->user_data;
    struct alloc_info* next_data = next->user_data;
    if (current_data->offset + current_data->size == next_data->offset) {
      current_data->size += next_data->size;
      dbll_remove(p->free_list, next);
    }
  }
}
