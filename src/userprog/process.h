#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
void argument_passing (char *file_name, void **esp);
struct thread *find_child(tid_t tid);

/* load() helpers. */
static bool install_page (void *upage, void *kpage, bool writable);

#endif /* userprog/process.h */
