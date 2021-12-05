#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "vm/swap.h"
#include "vm/frame.h"
#include "userprog/pagedir.h"
#include <list.h>

struct list_elem *clock_iter;

void frame_init(void)
{
  lock_init(&frame_lock);
  list_init(&frame_table);
  clock_iter = NULL;
}

struct frame *frame_create(enum palloc_flags flag)
{
  struct frame *new_frame = malloc(sizeof(struct frame));

  if(new_frame == NULL)
  {
    return NULL;
  }
  new_frame->kernel_vaddr = palloc_get_page(PAL_USER | flag);
  if (new_frame->kernel_vaddr == NULL)
  {
    clock_algorithm();
    new_frame->kernel_vaddr = palloc_get_page(PAL_USER | flag);
    new_frame->clock_bit = true;
  }
  new_frame->page = NULL;
  list_push_back(&frame_table, &new_frame->elem);

  return new_frame;
}

void frame_destroy(struct frame *frame)
{
  struct list_elem *e;
  if (frame == NULL)
  {
    return;
  }
  lock_acquire(&frame_lock);
  for(e = list_begin(&frame_table); e != list_end(&frame_table); e = list_next(e))
  {
    if(list_entry(e, struct frame, elem) == frame)
    {
      list_remove(e);
    }
  }
  palloc_free_page(frame->kernel_vaddr);
  free(frame);
  lock_release(&frame_lock);
}

bool set_page_frame(struct page *page)
{
  struct frame *frame;
  frame = page->type == ALL_ZERO ? frame_create(PAL_ZERO) : frame_create(PAL_USER);

  if(frame == NULL)
  {
    return false;
  }
  lock_acquire(&frame_lock);
  frame->page = page;
  frame->bit_index = -1;
  frame->clock_bit = true;
  page->frame = frame;
  lock_release(&frame_lock);

  return true;
}

void clock_algorithm(void)
{
  struct frame *cur_frame;
  struct page *cur_page;
  struct thread *cur = thread_current();
  int bit_index;
  bool dirty;

  while(1)
  {
    if(clock_iter == NULL || clock_iter == list_end(&frame_table))
    {
      clock_iter = list_begin(&frame_table);
    }
    else
    {
      clock_iter = list_next(clock_iter);
    }

    cur_frame = list_entry(clock_iter, struct frame, elem);
    if(cur_frame->clock_bit == true)
    {
      cur_frame->clock_bit = false;
    }
    else
    {
      break; 
    }
  }
  cur_page = cur_frame->page;
  bit_index = swap_out(cur_frame->kernel_vaddr);
  dirty = pagedir_is_dirty(cur->pagedir, cur_page->vaddr);
  set_swap_spt(cur_page, bit_index, dirty);
  palloc_free_page(cur_frame->kernel_vaddr);
}