#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "vm/frame.h"
#include "filesys/off_t.h"

enum page_type {
    ALL_ZERO,
    SWAP,
    FILE
};

struct page {
    struct frame *frame;    /* connect virtual address to physical address */ 
    void *vaddr;            /* page's virtual address(key) - palloc */

    enum page_type type;

    struct hash_elem helem;  /* manage page as hash */

    /* file information(need to add) */
    struct file *file;      /* information about original file */
    off_t file_offset;
    uint32_t read_bytes;
    uint32_t zero_bytes;
    bool writable;

    bool dirty;             /* initialize 0, if write occurs, set dirty as 1 */
};

/* use hash struct(provided by stanford pintos document) */
unsigned page_hash (const struct hash_elem *p_, void *aux);
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux);
struct page *page_lookup (const void *address);

/* funtion definition */
void spt_hash_init ();
void page_destroy(struct page *page);
bool set_file_spt (uint8_t *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable);
bool set_all_zero_spt (uint8_t *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable);
bool set_swap_spt (uint8_t *upage, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable);

#endif