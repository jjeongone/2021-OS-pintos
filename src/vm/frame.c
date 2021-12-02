#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

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
    /* clock algorithm: swap */
    lock_acquire(&frame_lock);
    palloc_free_page(new_frame->kernel_vaddr);
    free(new_frame);
    lock_release(&frame_lock);
    return NULL;
  }
  new_frame->page = NULL;
  return new_frame;
}

void frame_destroy(struct frame *frame)
{
  if (frame == NULL)
  {
    return;
  }
  lock_acquire(&frame_lock);
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
  page->frame = frame;
  lock_release(&frame_lock);
  /* maybe swap or eviction? */

  return true;
}