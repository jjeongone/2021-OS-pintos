#include "vm/swap.h"
#include "threads/vaddr.h"

void swap_init(void)
{
    swap_block = block_get_role(BLOCK_SWAP);
    uint32_t size = block_size(swap_block);
    swap_table = bitmap_create(size);
    bitmap_set_all(swap_table, false);
    lock_init(&swap_lock);
}

/* disk -> memory */
void swap_in(struct page *page, int bit_index, void *kernel_vaddr, bool dirty)
{
    int i; 
    lock_acquire(&swap_lock);
    bitmap_set(swap_table, bit_index, false);

    if(dirty)
    {
        for(i = 0; i < PGSIZE / BLOCK_SECTOR_SIZE ; i++)
        {
            block_read(swap_block, bit_index * (PGSIZE / BLOCK_SECTOR_SIZE) + i, kernel_vaddr + (BLOCK_SECTOR_SIZE * i));
        }
    }
    lock_release(&swap_lock);
}

/* memory -> disk */
int swap_out(void *kernel_vaddr)
{
    int i; 
    int bit_index;
    lock_acquire(&swap_lock);
    bit_index = bitmap_scan(swap_table, 0, 1, false);
    bitmap_set(swap_table, bit_index, true);
    for(i = 0; i < PGSIZE / BLOCK_SECTOR_SIZE ; i++)
    {
        block_write(swap_block, bit_index * PGSIZE / BLOCK_SECTOR_SIZE + i, kernel_vaddr + BLOCK_SECTOR_SIZE * i);
    }
    lock_release(&swap_lock);
    return bit_index;
}

void swap_destroy(uint32_t swap_index)
{
    bitmap_set(swap_table, swap_index, false);
}