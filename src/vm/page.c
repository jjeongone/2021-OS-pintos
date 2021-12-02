#include "vm/page.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"

#define MAX_STACK_SIZE 0x800000

/* Returns a hash value for page p. */
unsigned page_hash (const struct hash_elem *p_, void *aux)
{
  const struct page *p = hash_entry (p_, struct page, helem);
  return hash_bytes (&p->vaddr, sizeof p->vaddr);
}

/* Returns true if page a precedes page b. */
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux)
{
  const struct page *a = hash_entry (a_, struct page, helem);
  const struct page *b = hash_entry (b_, struct page, helem);

  return a->vaddr < b->vaddr;
}
 	
/* Returns the page containing the given virtual address,
   or a null pointer if no such page exists. */
struct page *page_lookup (void *address)
{
  struct page *p = (struct page *)malloc(sizeof(struct page));
  struct hash_elem *e;
  struct thread *cur = thread_current();

  p->vaddr = address;
  e = hash_find (cur->spt, &p->helem);
  free(p);
  return e != NULL ? hash_entry (e, struct page, helem) : NULL;
}

void spt_hash_init (void)
{
  thread_current()->spt = (struct hash *)malloc(sizeof(struct hash));
  hash_init(thread_current()->spt, page_hash, page_less, NULL);
}

void spt_hash_destroy(void)
{
  struct hash *h = thread_current()->spt;
  size_t i;

  for (i = 0; i < h->bucket_cnt; i++) 
  {
    struct list *bucket = &h->buckets[i];
    while (!list_empty (bucket)) 
    {
      struct list_elem *list_elem = list_pop_front (bucket);
      struct hash_elem *hash_elem = list_elem_to_hash_elem (list_elem);
      page_destroy(hash_entry(hash_elem, struct page, helem));
    }
    list_init (bucket); 
  }
  free(h->buckets);
}

void page_destroy(struct page *page)
{
  if (page == NULL)
  {
    return;
  }
  hash_delete(thread_current()->spt, &page->helem);
  free(page);
}

bool set_file_spt (uint8_t *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  struct thread *cur = thread_current();
  struct page *new_page = (struct page *)malloc(sizeof(struct page));

  if(new_page == NULL)
  {
    return false;
  }
  
  new_page->vaddr = upage;
  new_page->type = FILE;
  new_page->file = file;
  new_page->file_offset = ofs;
  new_page->read_bytes = read_bytes;
  new_page->zero_bytes = zero_bytes;
  new_page->writable = writable;
  new_page->dirty = false;
  new_page->frame = NULL;

  if(hash_insert(cur->spt, &new_page->helem) == NULL)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool set_all_zero_spt (uint8_t *upage)
{
  struct thread *cur = thread_current();
  struct page *new_page = malloc(sizeof(struct page));
  if(new_page == NULL)
  {
    return false;
  }

  new_page->vaddr = upage; 
  new_page->type = ALL_ZERO;
  new_page->file = NULL;
  new_page->file_offset = -1;
  new_page->read_bytes = -1;
  new_page->zero_bytes = -1;
  new_page->writable = false;
  new_page->dirty = false;
  new_page->frame = NULL;

  if(hash_insert(cur->spt, &new_page->helem) == NULL)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool set_swap_spt (uint8_t *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  return true;
}

bool check_stack_growth(struct intr_frame *f, void *fault_addr, void *initial_addr, bool user)
{
  void *cur_esp = user ? f->esp : thread_current()->esp;

  if(((fault_addr >= PHYS_BASE - MAX_STACK_SIZE && fault_addr < PHYS_BASE) && (fault_addr >= cur_esp || fault_addr == f->esp - 4 || fault_addr == f->esp - 32)))
  {
    if(page_lookup(initial_addr) == NULL)
    {
      set_all_zero_spt(initial_addr);
      return true;
    }
  }
  return false;
}
