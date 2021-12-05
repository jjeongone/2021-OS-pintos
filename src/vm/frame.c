#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
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
    // printf("clock kernel_vaddr: %p\n", new_frame->kernel_vaddr);
    new_frame->clock_bit = true;
  }
  // new_frame->page = NULL;

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
  lock_acquire(&frame_lock);
  frame = page->type == ALL_ZERO ? frame_create(PAL_ZERO) : frame_create(PAL_USER);

  if(frame == NULL)
  {
    return false;
  }
  frame->page = page;
  // frame->bit_index = -1;
  frame->clock_bit = true;
  frame->thread = thread_current();
  page->frame = frame;
  list_push_back(&frame_table, &frame->elem);
  lock_release(&frame_lock);
  // printf("page_type: %d, kernel_vaddr: %p, vaddr: %p\n", (int)page->type, frame->kernel_vaddr, page->vaddr);
  return true;
}

void clock_algorithm(void)
{
  struct frame *cur_frame;
  struct page *cur_page;
  int bit_index;
  bool dirty;

  while(1)
  {
    if(clock_iter == NULL || list_next(clock_iter) == list_end(&frame_table))
    {
      clock_iter = list_begin(&frame_table);
    }
    else
    {
      clock_iter = list_next(clock_iter);
    }

    cur_frame = list_entry(clock_iter, struct frame, elem);
    ASSERT(cur_frame->page != NULL);

    if(cur_frame->clock_bit == true)
    {
      cur_frame->clock_bit = false;
    }
    else
    {
      break; 
    }
  }
  // pagedir_clear_page(cur_frame->thread->pagedir, cur_frame->kernel_vaddr);
  cur_page = cur_frame->page;
  // printf("evict) kernel_vaddr: %p, vaddr: %p\n", cur_frame->kernel_vaddr, cur_page->vaddr);
  pagedir_clear_page(cur_frame->thread->pagedir, cur_page->vaddr);
  bit_index = swap_out(cur_frame->kernel_vaddr);
  dirty = pagedir_is_dirty(cur_frame->thread->pagedir, cur_frame->kernel_vaddr);
  set_swap_spt(cur_page, bit_index, dirty);

  palloc_free_page(cur_frame->kernel_vaddr);
  list_remove(clock_iter);
}