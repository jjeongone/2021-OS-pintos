#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/synch.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "lib/string.h"
#include "threads/palloc.h"
#include "vm/page.h"
#include "threads/malloc.h"

#define CODE_SEGMENT 0x8048000
#define off_t int

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void check_address (void *addr)
{
  if(addr == NULL || !is_user_vaddr(addr) || addr < (void *)CODE_SEGMENT)
  {
    sys_exit(-1);
  }
}

// void check_buffer_address (const void *addr, unsigned size)
// {
//   struct page *cur_page;
//   int i;
//   for(i = 0; i < size; i++)
//   {
//     check_address((void *)addr);
//     cur_page = page_lookup((void *)pg_round_down(addr));
//     // if(cur_page != NULL)
//     // {
//     //   if(!cur_page->writable)
//     //   {
//     //     sys_exit(-1);
//     //   }
//     // }
//     addr += 1;
//   }

//   // if(addr == NULL || !is_user_vaddr(addr) || cur_page == NULL || !cur_page->writable)
//   // // if(addr == NULL || !is_user_vaddr(addr))
//   // {
//   //   sys_exit(-1);
//   // }
// }

void check_buffer_address (const void *addr)
{
  if(addr == NULL || !is_user_vaddr(addr))
  {
    sys_exit(-1);
  }
}

void check_file_address (const char *file)
{
  if(file == NULL || !is_user_vaddr(file))
  {
    sys_exit(-1);
  }
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
int get_user (uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
bool put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

void sys_halt(void)
{
  shutdown_power_off();
}

void sys_exit(int status)
{
  struct thread *cur = thread_current ();

  printf("%s: exit(%d)\n", cur->name, status);
  cur->exit_status = status;
  thread_exit();
}

pid_t sys_exec(const char *cmd_line)
{
  pid_t pid;
  struct thread *child;

  if(!is_user_vaddr(cmd_line))
  {
    sys_exit(-1);
  }
  
  pid = process_execute(cmd_line);
  if(pid == TID_ERROR)
  {
    return -1;
  }
  
  child = find_child(pid);
  if(child == NULL)
  {
    return -1;
  }

  return child->is_load_success ? pid : -1;
}

int sys_wait (pid_t pid)
{
  return process_wait(pid);
}

bool sys_create (const char *file, unsigned initial_size)
{
  bool success;

  check_file_address(file);
  lock_acquire(&file_lock);
  success = filesys_create(file, initial_size);
  lock_release(&file_lock);
  return success;
}

bool sys_remove (const char *file)
{
  bool success;

  check_file_address(file);
  lock_acquire(&file_lock);
  success = filesys_remove(file);
  lock_release(&file_lock);
  return success;
}

int sys_open (const char *file)
{
  struct file *open_file;
  struct file_desc *new_fd = palloc_get_page(0);
  struct thread *cur = thread_current();
  
  if(new_fd == NULL)
  {
    return -1;
  }

  check_file_address(file);
  lock_acquire(&file_lock);

  open_file = filesys_open(file);
  if(open_file == NULL)
  {
    palloc_free_page(new_fd);
    lock_release(&file_lock);
    return -1;
  }
  else
  {
    if(strcmp(cur->name, file) == 0)
    {
      file_deny_write(open_file);
    }
    cur->fd_max = cur->fd_max + 1;
    new_fd->fd = cur->fd_max;
    new_fd->file = open_file;
    list_push_back(&cur->fd_list, &new_fd->felem);
    lock_release(&file_lock);
    return new_fd->fd;
  }
}

int sys_filesize (int fd)
{
  struct file_desc* open_file;

  lock_acquire(&file_lock);
  open_file = get_file_desc(fd);
  if(open_file == NULL || open_file->file == NULL)
  {
    lock_release(&file_lock);
    return -1;
  }
  lock_release(&file_lock);
  return file_length(open_file->file);
}

int sys_read (int fd, void *buffer, unsigned size)
{
  int key;
  unsigned i;
  int read_size;
  struct file_desc* open_file;

  check_address(buffer);
  lock_acquire(&file_lock);

  if(fd == 0)
  {
    for(i = 0; i < size; i++)
    {
      key = put_user(buffer + i, input_getc());
      if(!key)
      {
        lock_release(&file_lock);
        sys_exit(-1);
      }
    }
    lock_release(&file_lock);
    return size;
  }
  else
  {
    open_file = get_file_desc(fd);
    if(open_file == NULL || open_file->file == NULL)
    {
      lock_release(&file_lock);
      return -1;
    }
    read_size = file_read(open_file->file, buffer, size);
    lock_release(&file_lock);
    return read_size;
  }
}

int sys_write (int fd, const void *buffer, unsigned size)
{
  int write_size;
  struct file_desc* open_file;
  unsigned console_size;
  struct thread *cur = thread_current();

  check_buffer_address(buffer);
  lock_acquire(&file_lock);
  
  if(fd == 1)
  {
    console_size = size > 500 ? 500 : size;
    putbuf(buffer, console_size);
    lock_release(&file_lock);
    return console_size;
  }
  else
  {
    open_file = get_file_desc(fd);
    if(open_file == NULL || open_file->file == NULL)
    {
      lock_release(&file_lock);
      return -1;
    }
    write_size = file_write(open_file->file, buffer, size);
    lock_release(&file_lock);
    return write_size;
  }
}

void sys_seek (int fd, unsigned position)
{
  struct file_desc* open_file;

  lock_acquire(&file_lock);
  open_file = get_file_desc(fd);
  if(open_file == NULL || open_file->file == NULL)
  {
    lock_release(&file_lock);
    sys_exit(-1);
  }
  file_seek(open_file->file, position);
  lock_release(&file_lock);
}

unsigned sys_tell (int fd)
{
  unsigned next_pos;
  struct file_desc* open_file;

  lock_acquire(&file_lock);
  open_file = get_file_desc(fd);
  if(open_file == NULL || open_file->file == NULL)
  {
    lock_release(&file_lock);
    sys_exit(-1);
  }
  next_pos = file_tell(open_file->file);
  lock_release(&file_lock);

  return next_pos;
}

void sys_close (int fd)
{
  struct file_desc* open_file;

  lock_acquire(&file_lock);
  open_file = get_file_desc(fd);
  if(open_file == NULL || open_file->file == NULL)
  {
    lock_release(&file_lock);
    sys_exit(-1);
  }
  file_close(open_file->file);
  remove_file_desc(fd);
  lock_release(&file_lock);
}

mapid_t mmap (int fd, void *addr)
{
  struct file_desc *open_file;
  struct file *cur_file;
  struct page *mpage = (struct page *)malloc(sizeof(struct page));
  struct mmap_file *mmap = (struct mmap_file *)malloc(sizeof(struct mmap_file));
  struct thread *cur = thread_current();
  void *new_addr;
  int size, i, new_pos;
  int page_num, page_read_bytes, page_zero_bytes;

  if(addr == 0 || fd == 0 || fd == 1)
  {
    free(mpage);
    free(mmap);
    return -1;
  }

  open_file = get_file_desc(fd);

  if(open_file == NULL || open_file->file == NULL)
  {
    free(mpage);
    free(mmap);
    return -1;
  }
  
  cur_file = open_file->file;
  size = file_length(cur_file);

  if(size == 0)
  {
    free(mpage);
    free(mmap);
    return -1;
  }

  page_num = size / PGSIZE + 1;
  file_reopen(cur_file);

  for(i = 0; i < page_num; i++)
  {
    new_addr = addr + PGSIZE * i;
    if(page_lookup((void *)pg_round_down(new_addr)) != NULL)
    {
      free(mpage);
      free(mmap);
      return -1;
    }
  }

  mmap->vaddr = addr;
  mmap->id = cur->id_max;
  mmap->file = cur_file;
  cur->id_max = cur->id_max + 1;
  list_init(&mmap->mmap_page);

  for(i = 0; i < page_num; i++)
  {
    page_read_bytes = size - PGSIZE * i < PGSIZE ? size - PGSIZE * i : PGSIZE;
    page_zero_bytes = PGSIZE - page_read_bytes;
    new_addr = addr + PGSIZE * i;
    new_pos = file_tell(cur_file) + PGSIZE * i;
    
    set_file_spt(new_addr, cur_file, new_pos, page_read_bytes, page_zero_bytes, true); // check
    // mpage = page_lookup(new_addr);
    // set_page_frame(mpage); // exception.c?
    list_push_back(&mmap->mmap_page, &mpage->pelem);
  }

  // memset(mpage->frame->kernel_vaddr, 0, page_zero_bytes); // exception.c?
  list_push_back(&cur->mmap_list, &mmap->melem);
  free(mpage);
  return mmap->id;
}

void munmap(mapid_t mapping)
{
  struct thread *cur = thread_current();
  struct list_elem *e;
  struct list_elem *p;
  struct mmap_file *cur_map;
  struct page *cur_page;

  for(e = list_begin(&cur->mmap_list); e != list_end(&cur->mmap_list); e = list_next(e))
  {
    cur_map = list_entry(e, struct mmap_file, melem);
    if(cur_map->id == mapping)
    {
      for(p = list_begin(&cur_map->mmap_page); p != list_end(&cur_map->mmap_page); p = list_next(p))
      {
        // write back
        cur_page = list_entry(p, struct page, pelem);
        if(pagedir_is_dirty(cur->pagedir, cur_page->frame->kernel_vaddr))
        {
          file_write_at(cur_page->file, cur_page->frame->kernel_vaddr, cur_page->read_bytes, cur_page->file_offset);
        }
        page_destroy(cur_page);
      }
      file_close(cur_map->file);
    }
    free(cur_map);
  }
}

struct file_desc* get_file_desc(int fd)
{
  struct thread *cur = thread_current();
  struct list_elem *e;

  for(e = list_begin(&cur->fd_list); e != list_end(&cur->fd_list); e = list_next(e))
  {
    if(list_entry(e, struct file_desc, felem)->fd == fd)
    {
      return list_entry(e, struct file_desc, felem);
    }
  }
  return NULL;
}

void remove_file_desc(int fd)
{
  struct thread *cur = thread_current();
  struct list_elem *e;
  
  for(e = list_begin(&cur->fd_list); e != list_end(&cur->fd_list); e = list_next(e))
  {
    if(list_entry(e, struct file_desc, felem)->fd == fd)
    {
      list_remove(e);
    }
  }
}

void get_stack_argument (void *esp, int byte_size, void *argument)
{
  int i;

  for(i = 0; i < byte_size; i++)
  {
    check_address(esp + i);
    *(char *)(argument + i) = get_user(esp + i);
  }
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int syscall_num;
  int arg0;
  int arg1;
  void *arg2;

  check_address((void *)f->esp);
  thread_current()->esp = f->esp;
  get_stack_argument(f->esp, 4, &syscall_num);

  switch (syscall_num)
  {
    case SYS_HALT:
      sys_halt();
      break;
    case SYS_EXIT:
      get_stack_argument(f->esp + 4, 4, &arg0);
      sys_exit((int)arg0);
      break;
    case SYS_EXEC:
      get_stack_argument(f->esp + 4, 4, &arg2);
      f->eax = sys_exec((const char *)arg2);
      break;
    case SYS_WAIT:
      get_stack_argument(f->esp + 4, 4, &arg0);
      f->eax = sys_wait((pid_t)arg0);
      break;
    case SYS_CREATE:
      get_stack_argument(f->esp + 4, 4, &arg2);
      get_stack_argument(f->esp + 8, 4, &arg0);
      f->eax = sys_create((const char *)arg2, (unsigned)arg0);
      break;
    case SYS_REMOVE:
      get_stack_argument(f->esp + 4, 4, &arg2);
      f->eax = sys_remove((const char *)arg2);
      break;
    case SYS_OPEN:
      get_stack_argument(f->esp + 4, 4, &arg2);
      f->eax = sys_open((const char *)arg2);
      break;
    case SYS_FILESIZE:
      get_stack_argument(f->esp + 4, 4, &arg0);
      f->eax = sys_filesize((int)arg0);
      break;
    case SYS_READ:
      get_stack_argument(f->esp + 4, 4, &arg0);
      get_stack_argument(f->esp + 8, 4, &arg2);
      get_stack_argument(f->esp + 12, 4, &arg1);
      f->eax = sys_read((int)arg0, arg2, (unsigned)arg1);
      break;
    case SYS_WRITE:
      get_stack_argument(f->esp + 4, 4, &arg0);
      get_stack_argument(f->esp + 8, 4, &arg2);
      get_stack_argument(f->esp + 12, 4, &arg1);
      f->eax = sys_write((int)arg0, arg2, (unsigned)arg1);
      break;
    case SYS_SEEK:
      get_stack_argument(f->esp + 4, 4, &arg0);
      get_stack_argument(f->esp + 8, 4, &arg1);
      sys_seek((int)arg0, (unsigned)arg1);
      break;
    case SYS_TELL:
      get_stack_argument(f->esp + 4, 4, &arg0);
      f->eax = sys_tell((int)arg0);
      break;
    case SYS_CLOSE:
      get_stack_argument(f->esp + 4, 4, &arg0);
      sys_close((int)arg0);
      break;
    case SYS_MMAP:
      get_stack_argument(f->esp + 4, 4, &arg0);
      get_stack_argument(f->esp + 8, 4, &arg2);
      f->eax = mmap((int)arg0, arg2);
      break;
    case SYS_MUNMAP:
      get_stack_argument(f->esp + 4, 4, &arg0);
      munmap((int)arg0);
      break;
  }
}
