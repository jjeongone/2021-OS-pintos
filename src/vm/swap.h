#ifndef VM_SWAP_H
#define VM_SWAP_H
#include "devices/block.h"
#include "threads/synch.h"
#include "vm/page.h"
#include <bitmap.h>

struct block *swap_block;
struct bitmap *swap_table;
struct lock swap_lock;

void swap_init(void);
void swap_in(struct page *page, int bit_index, void *kernel_vaddr, bool dirty);
int swap_out(void *kernel_vaddr);
void swap_destroy(uint32_t swap_index);

#endif