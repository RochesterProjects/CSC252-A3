#include <stdlib.h>
#include <stdio.h>
#include "dbll.h"

/* Routines to create and manipulate a doubly-linked list */

/* create a doubly-linked list */
/* returns an empty list or NULL if memory allocation failed */
struct dbll *dbll_create()
{
  struct dbll* this = (struct dbll*)malloc(sizeof(struct dbll));
  if(this == NULL){
    return NULL;
  }
  //struct llnode *toFirst = (struct llnode*)malloc(sizeof(struct llnode));
 // struct llnode *toLast = (struct llnode*)malloc(sizeof(struct llnode));
  this->first = NULL;
  this->last = NULL;

  return this;
}

/* frees all memory associated with a doubly-linked list */
/* this must also free all memory associated with the linked list nodes */
/* assumes user data has already been freed */
void dbll_free(struct dbll *list)
{
  struct llnode *curr = list->first;
  while(curr != NULL){
    struct llnode *next = curr->next;
    free(curr);
    curr = next;
  }
  free(list);
}

/* iterate over items stored in a doubly-linked list */

/* begin iteration from node start, end iteration at node stop (include stop in iteration) */
/* the list is traversed using the next pointer of each node */

/* if start is NULL, then start is the first node */
/* if end is NULL, then end is the last node */

/* at each node, call the user function f with a pointer to the list,
   a pointer to the current node, and the value of ctx */

/* if f returns 0, stop iteration and return 1 */

/* return 0 if you reached the end of the list without encountering end */
/* return 1 on successful iteration */

int dbll_iterate(struct dbll *list,
				 struct llnode *start,
				 struct llnode *end,
				 void *ctx,
				 int (*f)(struct dbll *, struct llnode *, void *))
{
  if(start == NULL){
    start = list->first;
  }
  if(end == NULL){
    end = list->last;
  }
 struct llnode *curr = start;
 int result;
  while(curr != end){
    if(f != NULL){
      result = (*f)(list,curr,ctx);
      if(result == 0){
        return 1;
      }
    }
    if(&curr == &(list->last)){
      return 0;
    }
    curr = curr->next;
    }
    if(f != NULL){
    result = f(list,curr,ctx);
    if(result == 0){
      return 1;
    }
  }
    return 1;
    
  }
  


/* similar to dbll_iterate, except that the list is traversed using
   the prev pointer of each node (i.e. in the reverse direction).

   Adjust accordingly. For example if start is NULL, then start is the
   last node in the list.  If end is NULL, then end is the first node
   in the list.

*/

int dbll_iterate_reverse(struct dbll *list,
						 struct llnode *start,
						 struct llnode *end,
						 void *ctx,
						 int (*f)(struct dbll *, struct llnode *, void *)
						 )
{
  if(start == NULL){
    start = list->last;
  }
  if(end == NULL){
    end = list->first;
  }
  struct llnode *curr = start;
  int result;
  while(curr != end){
    if(f != NULL){
      result = (*f)(list,curr,ctx);
      if(result == 0){
        return 1;
      }
    }
    if(&curr == &(list->first)){
      return 0;
    }
    curr = curr->prev;
    }
    if(f != NULL){
    result = f(list,curr,ctx);
    if(result == 0){
      return 1;
    }
  }
  return 1;
}


/* Remove `llnode` from `list` */
/* Memory associated with `node` must be freed */
/* You can assume user_data will be freed by somebody else (or has already been freed) */
void dbll_remove(struct dbll *list, struct llnode *node)
{
  struct llnode* pprev = node->prev;
  struct llnode* pnext = node->next;

  if(pprev != NULL){
    pprev->next = pnext;
  }
  else{
    list->first = pnext;
  }

  if(pnext != NULL){
    pnext->prev = pprev;
  }
  else{
    list->last = pprev;
  }
  free(node);

  
}

/* Create and return a new node containing `user_data` */
/* The new node must be inserted after `node` */
/* if node is NULL, then insert the node at the end of the list */
/* return NULL if memory could not be allocated */
struct llnode *dbll_insert_after(struct dbll *list, struct llnode *node, void *user_data)
{
   struct llnode *toInsert = malloc(sizeof(struct llnode));
  if(toInsert == NULL){
    return NULL;
  }
  toInsert->user_data = user_data;

  if(node != NULL){
    struct llnode *pnext = node->next;
    toInsert->next = pnext;
    node->next = toInsert;
    toInsert->prev = node;
    if(pnext == NULL){
      list->last = toInsert;
    }}else{
      list->last = toInsert;
      if(list->first == NULL){
        list->first = toInsert;
      }
    }
  
  return toInsert;
}

/* Create and return a new node containing `user_data` */
/* The new node must be inserted before `node` */
/* if node is NULL, then insert the new node at the beginning of the list */
/* return NULL if memory could not be allocated */
struct llnode *dbll_insert_before(struct dbll *list, struct llnode *node, void *user_data)
{
  struct llnode *toInsert = malloc(sizeof(struct llnode));
  if(toInsert == NULL){
    return NULL;
  }
  
  toInsert->user_data = user_data;

  if(node != NULL){
    struct llnode *pprev = node->prev;
    toInsert->prev = pprev;
    node->prev = toInsert;
    toInsert->next = node;

    if(pprev == NULL){list->first = toInsert;}
  }else{
      list->first = toInsert;
      if(list->last == NULL){
        list->last = toInsert;
      }
    }
  
 
  return toInsert;
}

/* create and return an `llnode` that stores `user_data` */
/* the `llnode` is added to the end of list */
/* return NULL if memory could not be allocated */
/* this function is a convenience function and can use the dbll_insert_after function */
struct llnode *dbll_append(struct dbll *list, void *user_data)
{ 
  struct llnode *toAppend = (struct llnode*)malloc(sizeof(struct llnode));
  if(toAppend == NULL){
    return NULL;

  }
  toAppend->user_data = user_data;
  toAppend->next = NULL;
  toAppend->prev = list->last;
  if(list->last != NULL){
    list->last->next = toAppend;
  }
  list->last = toAppend;
  if(list->first == NULL){
    list->first = toAppend;
  }
  return toAppend;
 
}
