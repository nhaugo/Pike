/*
|| This file is part of Pike. For copyright information see COPYRIGHT.
|| Pike is distributed under GPL, LGPL and MPL. See the file COPYING
|| for more information.
|| $Id: callback.c,v 1.31 2002/11/23 13:27:27 mast Exp $
*/

#include "global.h"
#include "pike_macros.h"
#include "callback.h"
#include "pike_error.h"
#include "block_alloc.h"

RCSID("$Id: callback.c,v 1.31 2002/11/23 13:27:27 mast Exp $");

struct callback_list fork_child_callback;

/*
 * This file is used to simplify the management of callbacks when certain
 * events occur. The callbacks are managed as linked lists, allocated in
 * chunks.
 */

/* FIXME: free all chunks of memory at exit */

struct callback
{
  struct callback *next;
  callback_func call;
  callback_func free_func;
  void *arg;
};

#define CALLBACK_CHUNK 128
#ifdef PIKE_DEBUG
#undef PRE_INIT_BLOCK
#define PRE_INIT_BLOCK(X) X->free_func=(callback_func)remove_callback;
#endif
BLOCK_ALLOC(callback, CALLBACK_CHUNK)


#ifdef PIKE_DEBUG
extern int d_flag;

static int is_in_free_list(struct callback * c)
{
  struct callback_block *bar;
  int e;

  if (!c) return 0;

  for (bar = callback_blocks; bar; bar=bar->next) {
    if ((bar->x <= c) && ((c - bar->x) < CALLBACK_CHUNK)) {
      struct callback *foo;
      for (foo = bar->free_callbacks; foo;
	   foo = (void *)foo->BLOCK_ALLOC_NEXT) {
	if (foo == c) return 1;
      }
      return 0;
    }
  }

  return 0;
}

static void check_callback_chain(struct callback_list *lst)
{
  int e,len=0;
  struct callback_block *tmp;
  struct callback *foo;
  if(d_flag>4)
  {
    for(foo=lst->callbacks;foo;foo=foo->next)
    {
      if((len & 1024)==1023)
      {
	int len2=0;
	struct callback *tmp;
	for(tmp=foo->next;tmp && len2<=len;tmp=tmp->next)
	{
	  if(tmp==foo)
	    Pike_fatal("Callback list is cyclic!!!\n");
	}
      }
      len++;
    }
    
    for(tmp=callback_blocks;tmp;tmp=tmp->next)
    {
      for(e=0;e<CALLBACK_CHUNK;e++)
      {
	int d;
	struct callback_block *tmp2;
	
	if(tmp->x[e].free_func == (callback_func)remove_callback)
	{
	  if(!is_in_free_list(tmp->x+e))
	    Pike_fatal("Lost track of a struct callback!\n");

	  if(tmp->x[e].next &&
	     !is_in_free_list(tmp->x[e].next))
	    Pike_fatal("Free callback has next in Z'ha'dum!\n");

	}else{
	  if(is_in_free_list(tmp->x[e].next))
	    Pike_fatal("Non-free callback has next in free list!\n");
	}
	
	if(tmp->x[e].next)
	{
	  d=CALLBACK_CHUNK;
	  for(tmp2=callback_blocks;tmp2;tmp2=tmp2->next)
	  {
	    for(d=0;d<CALLBACK_CHUNK;d++)
	    {
	      if(tmp2->x+d == tmp->x[e].next)
		break;
	      
	      if(d < CALLBACK_CHUNK) break;
	    }
	  }
	  
	  if(d == CALLBACK_CHUNK)
	    Pike_fatal("Callback next pointer pointing to Z'ha'dum\n");
	}
      }
    }
  }
}
#else
#define check_callback_chain(X)
#endif

/* Return the first free callback struct, allocate more if needed */


/* Traverse a linked list of callbacks and call all the active callbacks
 * in the list. Deactivated callbacks are freed and placed in the free list.
 */
PMOD_EXPORT void low_call_callback(struct callback_list *lst, void *arg)
{
  int this_call;
  struct callback *l,**ptr;

  lst->num_calls++;
  this_call=lst->num_calls;

  check_callback_chain(lst);
  ptr=&lst->callbacks;
  while((l=*ptr))
  {
    if(l->call)
    {
      l->call(l,l->arg, arg);
      if(lst->num_calls != this_call) return;
    }

    if(!l->call)
    {
      if(l->free_func)
	l->free_func(l, l->arg, 0);

      while(*ptr != l)
      {
	ptr=&(ptr[0]->next);
	if(!*ptr)
	{
	  /* We totally failed to find where we are in the linked list.. */
	  Pike_fatal("Callback linked list breakdown.\n");
	}
      }

      *ptr=l->next;
#ifdef PIKE_DEBUG
      l->free_func=(callback_func)remove_callback;
#endif
      really_free_callback(l);
    }else{
      ptr=& l->next;
    }
    check_callback_chain(lst);
  }
}

/* Add a callback to the linked list pointed to by ptr. */
PMOD_EXPORT struct callback *debug_add_to_callback(struct callback_list *lst,
				       callback_func call,
				       void *arg,
				       callback_func free_func)
{
  struct callback *l;
  l=alloc_callback();
  l->call=call;
  l->arg=arg;
  l->free_func=free_func;

  DO_IF_DMALLOC( if(l->free_func == (callback_func)free)
		 l->free_func=(callback_func)dmalloc_free; )

  l->next=lst->callbacks;
  lst->callbacks=l;

  check_callback_chain(lst);

  return l;
}

/* This function deactivates a callback.
 * It is not actually freed until next time this callback is "called"
 */
PMOD_EXPORT void *remove_callback(struct callback *l)
{
  dmalloc_unregister(l,1);
  l->call=0;
  l->free_func=0;
  return l->arg;
}

/* Free all the callbacks in a linked list of callbacks */
void free_callback_list(struct callback_list *lst)
{
  struct callback *l,**ptr;
  check_callback_chain(lst);
  ptr=& lst->callbacks;
  while((l=*ptr))
  {
    if(l->free_func)
      l->free_func(l, l->arg, 0);
    *ptr=l->next;
    really_free_callback(l);
  }
}

void cleanup_callbacks(void)
{
  free_all_callback_blocks();
}
