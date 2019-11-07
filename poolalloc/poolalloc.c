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

  /* create a doubly-linked list to track free blocks */
  mpool->free_list = dbll_create();

  /* create a free block of memory for the entire pool and place it on the free_list */
  struct alloc_info *mem_block = (struct alloc_info*) malloc(sizeof(struct alloc_info));
  mem_block->size = size;
  mem_block->offset = 0;
  mem_block->request_size = 0;
  dbll_append(mpool->free_list, mem_block);
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
    case 4:
      align = 4;
      break;
    case 5:
    case 6:
    case 7:
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
  struct llnode* node = NULL;
  for (struct llnode* cur = p->free_list->first; cur != NULL; cur = cur->next) {
    struct alloc_info* temp = cur->user_data;
    size_t start = temp->offset;
    if (start % align != 0)
      start = (start / align + 1) * align;
    if (temp->offset + temp->size >= start + size) {
      node = cur;
      break;
    }
  }

  /* if no suitable block can be found, return NULL */
  if (node == NULL)
    return NULL;
  struct alloc_info* node_data = node->user_data;

  /* if found, create an alloc_info node, store start of new region
   into offset, set size to allocation size (take alignment into
   account!), set free to null */
  struct alloc_info* block;
  if (node_data->offset % align != 0) {
    // Split memory block into 2 blocks: offset->alignment-1 and alignment->offset+size 
    block = malloc( sizeof(struct alloc_info) );
    block->size = (node_data->offset / align + 1) * align - node_data->offset;
    block->offset = node_data->offset;

    node_data->offset += block->size;
    node_data->size -= block->size;

    dbll_insert_before(p->free_list, node, block);
  }

  block = malloc( sizeof(struct alloc_info) );
  block->size = size;
  block->offset = node_data->offset;
  block->request_size = size;

  node_data->offset += block->size;
  node_data->size -= block->size;
  if (node_data->size == 0) 
    dbll_remove(p->free_list, node);

  /* add the new alloc_info node to the memory pool's allocated
   list */
  dbll_append(p->alloc_list, block);

  /* return pointer to allocated region*/
  return p->start + block->offset;

}

/* Free a chunk of memory out of the pool */
/* This moves the chunk of memory to the free list. */
/* You may want to coalesce free blocks [i.e. combine two free blocks
   that are are next to each other in the pool into one larger free
   block. Note this requires that you keep the list of free blocks in order */
void mpool_free(struct memory_pool *p, void *addr)
{
  /* search the alloc_list for the block */
  struct llnode* node = NULL;
  for (struct llnode* curr = p->alloc_list->first; curr != NULL; curr = curr->next) 
    if (((struct alloc_info*) curr->user_data)->offset + p->start == addr) {
      node = curr;
      break;
    }

  /* move it to the free_list */
  struct alloc_info* data = node->user_data;
  // Remove from alloc_list
  dbll_remove(p->alloc_list, node);

  // Add to free_list
  node = NULL;
  for (struct llnode* cur = p->free_list->first; cur != NULL; cur = cur->next) 
    if (((struct alloc_info*) cur->user_data)->offset > (size_t) addr - (size_t) p->start) {
      node = cur;
      break;
    }
  
  if (node != NULL)
    node = dbll_insert_before(p->free_list, node, data);
  else 
    node = dbll_append(p->free_list, data);

  /* coalesce the free_list */

  // coalesce the previous node
  if (node->prev != NULL) {
    struct llnode* prev = node->prev;
    struct alloc_info* current_data = node->user_data;
    struct alloc_info* prev_data = prev->user_data;
    if (prev_data->offset + prev_data->size == current_data->offset) {
      current_data->offset -= prev_data->size;
      current_data->size += prev_data->size;
      dbll_remove(p->free_list, prev);
    }
  }

  // coalesce the next node
  if  (node->next != NULL) {
    struct llnode* next = node->next;
    struct alloc_info* current_data = node->user_data;
    struct alloc_info* next_data = next->user_data;
    if (current_data->offset + current_data->size == next_data->offset) {
      current_data->size += next_data->size;
      dbll_remove(p->free_list, next);
    }
  }
}
