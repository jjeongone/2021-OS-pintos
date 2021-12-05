#ifndef VM_SWAP_H
#define VM_SWAP_H
#include "devices/block.h"
#include "vm/page.h"
#include <bitmap.h>

struct block *swap_block;
struct bitmap *swap_table;

void swap_init(void);
void swap_in(struct page *page, int bit_index, void *kernel_vaddr, bool dirty);
int swap_out(void *kernel_vaddr);
// static bool swap_in_file(struct page *page,  void *kernel_vaddr);
// static bool swap_out_file(struct page *page);

#endif