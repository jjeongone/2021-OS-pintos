#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "vm/page.h"
#include <hash.h>
#include "threads/palloc.h"

struct lock frame_lock;

struct frame {
    struct page *page;          /* mapped page table entry */
    void *kernel_vaddr;         /* actual physical address(physical address + PHYS_BASE) */
};

/* use hash struct(provided by stanford pintos document) */
// unsigned frame_hash (const struct hash_elem *p_, void *aux UNUSED);
// bool frame_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);
// struct page *frame_lookup (const void *address);

struct frame *frame_create(enum palloc_flags flag);
void frame_destroy(struct frame *frame);
bool set_page_frame(struct page *page);

#endif