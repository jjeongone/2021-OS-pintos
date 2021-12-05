#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "vm/page.h"
#include <hash.h>
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

struct lock frame_lock;
struct list frame_table;

struct frame {
    struct page *page;          /* mapped page table entry */
    void *kernel_vaddr;         /* actual physical address(physical address + PHYS_BASE) */
    // int bit_index;              /* where in the swap table */
    bool clock_bit;             /* for clock algorithm */
    
    struct thread *thread;
    
    struct list_elem elem;
};

void frame_init(void);
struct frame *frame_create(enum palloc_flags flag);
void frame_destroy(struct frame *frame);
bool set_page_frame(struct page *page); 
void clock_algorithm(void);

#endif