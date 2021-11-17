#ifndef VM_FRAME_H
#define VM_FRAME_H

struct frame {
    struct *page page;          /* mapped page table entry */
    void *kernel_vaddr;         /* actual physical address(physical address + PHYS_BASE) */
    
    struct *thread thread;      /* thread that owns frame */

    struct hash_elem helem;     /* manage frame as hash */
}

/* use hash struct(provided by stanford pintos document) */
unsigned frame_hash (const struct hash_elem *p_, void *aux UNUSED);
bool frame_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);
struct page *frame_lookup (const void *address);