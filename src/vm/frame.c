#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/palloc.h"

// struct hash frames;     /* need to initialization */

// /* Returns a hash value for frame f. */
// unsigned
// frame_hash (const struct hash_elem *p_, void *aux UNUSED)
// {
//   const struct frame *f = hash_entry (p_, struct frame, hash_elem);
//   return hash_bytes (&f->vaddr, sizeof f->vaddr);
// }

// /* Returns true if frame a precedes frame b. */
// bool
// frame_less (const struct hash_elem *a_, const struct hash_elem *b_,
//            void *aux UNUSED)
// {
//   const struct frame *a = hash_entry (a_, struct frame, hash_elem);
//   const struct frame *b = hash_entry (b_, struct frame, hash_elem);

//   return a->vaddr < b->vaddr;
// }
 	
// /* Returns the frame containing the given virtual address,
//    or a null pointer if no such frame exists. */
// struct frame *
// frame_lookup (const void *address)
// {
//   struct frame f;
//   struct hash_elem *e;

//   f.vaddr = address;
//   e = hash_find (&frames, &f.hash_elem);
//   return e != NULL ? hash_entry (e, struct frame, hash_elem) : NULL;
// }

struct frame *frame_create(void)
{
  struct frame *new_frame = malloc(sizeof(struct frame));
  new_frame->kernel_vaddr = palloc_get_page(PAL_USER);
  if (new_frame->kernel_vaddr == NULL)
  {
    free(new_frame);
    palloc_free_page(new_frame->kernel_vaddr);
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


/*
  called by load_segment
  1. allocate new page and frame
  2. load page by given varaibles
*/
bool page_init()
{

}