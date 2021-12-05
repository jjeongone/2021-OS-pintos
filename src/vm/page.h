#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "vm/frame.h"
#include "filesys/off_t.h"
#include "threads/interrupt.h"

enum page_type {
    ALL_ZERO,
    SWAP,
    FILE,
    FRAME
};

struct page {
    struct frame *frame;    /* connect virtual address to physical address */ 
    void *vaddr;            /* page's virtual address(key) - palloc */

    enum page_type type;

    struct hash_elem helem;  /* manage page as hash */
    struct list_elem pelem;  /* manage file mapped page list */

    /* file information(need to add) */
    struct file *file;      /* information about original file */
    off_t file_offset;
    uint32_t read_bytes;
    uint32_t zero_bytes;
    bool writable;

    int bit_index;              /* where in the swap table */

    bool dirty;             /* initialize 0, if write occurs, set dirty as 1 */
};

/* use hash struct(provided by stanford pintos document) */
unsigned page_hash (const struct hash_elem *p_, void *aux);
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux);
struct page *page_lookup (void *address);

/* funtion definition */
void spt_hash_init (void);
void spt_hash_destroy(void);
void page_destroy(struct page *page);
bool set_file_spt (uint8_t *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable);
bool set_all_zero_spt (uint8_t *upage);
bool set_swap_spt (struct page *page, int bit_index, bool dirty);
bool check_stack_growth(struct intr_frame *f, void *fault_addr, void *initial_addr, bool user);

#endif