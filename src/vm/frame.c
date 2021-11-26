#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"

struct lock frame_lock;

struct frame *frame_create(void)
{
  struct frame *new_frame = malloc(sizeof(struct frame));
  new_frame->kernel_vaddr = palloc_get_page(PAL_USER | PAL_ZERO);
  if (new_frame->kernel_vaddr == NULL)
  {
    palloc_free_page(new_frame->kernel_vaddr);
    free(new_frame);
    return NULL;
  }
  new_frame->page = NULL;

  /* need to be implemented */

  return new_frame;
}

void frame_destroy(struct frame *frame)
{
  if (frame == NULL)
  {
    return;
  }
  palloc_free_page(frame->kernel_vaddr);
  free(frame);
}

bool set_page_frame(struct page *page)
{
  struct frame *frame = frame_create();

  if(frame == NULL)
  {
    return false;
  }
  frame->page = page;
  page->frame = frame;

  /* maybe swap or eviction? */

  return true;
}