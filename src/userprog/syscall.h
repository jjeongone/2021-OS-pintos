#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>
#include "threads/synch.h"
#include "vm/page.h"

typedef int pid_t;
typedef int mapid_t;

struct file_desc
{
   int fd;
   struct file *file;
   struct list_elem felem;
};

struct mmap_file
{
   int id;
   void *vaddr;
   int size;
   struct page *page;

   struct list_elem melem;
   struct file *file;
   struct list page_list;
};

void syscall_init (void);
void check_address (void *addr);
void check_buffer_address (const void *addr);
void check_file_address (const char *file);
int get_user (uint8_t *uaddr);
bool put_user (uint8_t *udst, uint8_t byte);
void get_stack_argument (void *esp, int byte_size, void *argument);

void sys_halt (void);
void sys_exit (int status);
pid_t sys_exec (const char *cmd_line);
int sys_wait (pid_t pid);
bool sys_create (const char *file, unsigned initial_size);
bool sys_remove (const char *file);
int sys_open (const char *file);
int sys_filesize (int fd);
int sys_read (int fd, void *buffer, unsigned size);
int sys_write (int fd, const void *buffer, unsigned size);
void sys_seek (int fd, unsigned position);
unsigned sys_tell (int fd);
void sys_close (int fd);
mapid_t mmap (int fd, void *addr);
void munmap(mapid_t mapping);

struct file_desc* get_file_desc(int fd);
void remove_file_desc(int fd);

struct lock file_lock;

#endif /* userprog/syscall.h */
