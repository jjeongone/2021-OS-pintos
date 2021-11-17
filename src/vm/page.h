#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "vm/frame.h"

struct page {
    struct frame *frame;    /* connect virtual address to physical address */ 
    void *vaddr;            /* page's virtual address(key) - palloc */

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
unsigned page_hash (const struct hash_elem *p_, void *aux UNUSED);
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);
struct page *page_lookup (const void *address);

/* funtion definition */

#endif