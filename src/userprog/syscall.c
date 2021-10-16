#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);
static struct lock file_lock;

void
syscall_init (void) 
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void check_address (void *addr)
{
  if(addr == NULL || !is_user_vaddr(addr))
  {
    sys_exit(-1);
  }
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

void get_stack_argument (void *esp, int byte_size, int *argument)
{
  int i;
  uint8_t result;

  check_address(esp);
  for(i = 0; i < byte_size; i++)
  {
    result = get_user(esp + i);
    if(result == -1)
    {
      sys_exit(-1);
    }
    *(argument + i) = result;
  }
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int *syscall_num;
  int *arg0;
  int *arg1;
  int *arg2;

  printf ("system call!\n");
  check_address(f.esp);
  get_stack_argument(f.esp, 4, syscall_num);

  switch (syscall_num)
  {
    case SYS_HALT:
      sys_halt();
      break;
    case SYS_EXIT:
      get_stack_argument(f.esp + 4, 4, arg0);
      sys_exit(arg0);
      break;
    case SYS_EXEC:
      get_stack_argument(f.esp + 4, 4, arg0);
      f->eax = sys_exec(arg0);
      break;
    case SYS_WAIT:
      get_stack_argument(f.esp + 4, 4, arg0);
      f->eax = sys_wait(arg0);
      break;
    case SYS_CREATE:
      get_stack_argument(f.esp + 4, 4, arg0);
      get_stack_argument(f.esp + 8, 4, arg1);
      f->eax = sys_create(arg0, arg1);
      break;
    case SYS_REMOVE:
      get_stack_argument(f.esp + 4, 4, arg0);
      f->eax = sys_remove(arg0);
      break;
    case SYS_OPEN:
      get_stack_argument(f.esp + 4, 4, arg0);
      f->eax = sys_open(arg0);
      break;
    case SYS_FILESIZE:
      get_stack_argument(f.esp + 4, 4, arg0);
      f->eax = sys_filesize(arg0);
      break;
    case SYS_READ:
      get_stack_argument(f.esp + 4, 4, arg0);
      get_stack_argument(f.esp + 8, 4, arg1);
      get_stack_argument(f.esp + 12, 4, arg2);
      f->eax = sys_read(arg0, arg1, arg2);
      break;
    case SYS_WRITE:
      get_stack_argument(f.esp + 4, 4, arg0);
      get_stack_argument(f.esp + 8, 4, arg1);
      get_stack_argument(f.esp + 12, 4, arg2);
      f->eax = sys_write(arg0, arg1, arg2);
      break;
    case SYS_SEEK:
      get_stack_argument(f.esp + 4, 4, arg0);
      get_stack_argument(f.esp + 8, 4, arg1);
      sys_seek(arg0, arg1);
      break;
    case SYS_TELL:
      get_stack_argument(f.esp + 4, 4, arg0);
      f->eax = sys_tell(arg0);
      break;
    case SYS_CLOSE:
      get_stack_argument(f.esp + 4, 4, arg0);
      sys_close(arg0);
      break;
    default:
  }
  thread_exit ();
}

void sys_halt()
{
  shutdown_power_off();
}

void sys_exit(int status)
{
  struct thread *cur = thread_current ();
  struct list_elem *e;

  printf("%s: exit(%d)\n", cur->name, status);
  cur->exit_status = status;
  for(e = list_begin(&cur->fd_list); e != list_end(&cur->fd_list); e = list_next(e))
  {
    sys_close(list_entry(e, struct file_desc, felem)->fd);
  }
  thread_exit();
}

pid_t sys_exec(const char *cmd_line)
{
  pid_t pid = process_execute(cmd_line)
  int value;
  struct list_elem *e;
  struct thread *cur = thread_current();
  struct thread *child;

  if(pid == TID_ERROR)
  {
    return -1;
  }

  for(e = list_begin(&cur->child_list); e != list_end(&cur->child_list); e = list_next(e))
  {
    if(list_entry(e, struct thread, celem)->tid == pid)
    {
      child = list_entry(e, struct thread, celem);
      sema_down(&child->initial_sema);
    }
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

  check_address(file);
  lock_acquire(&file_lock);
  success = filesys_create(file, initial_size);
  lock_release(&file_lock);

  return success;
}

bool sys_remove (const char *file)
{
  bool success;

  check_address(file);
  lock_acquire(&file_lock);
  success = filesys_remove(file);
  lock_release(&file_lock)

  return success;
}

int sys_open (const char *file)
{
  struct file *open_file;
  struct file_desc *new_fd;
  
  check_address(file);
  lock_acquire(&file_lock);
  open_file = filesys_open(file);
  if(open_file == NULL)
  {
    lock_release(&file_lock);

    return -1;
  }
  else
  {
    file_deny_write(open_file->file);
    thread_current()->fd_max++;
    new_fd->fd = thread_current()->fd_max;
    new_fd->file = open_file;
    list_push_back(&thread_current->fd_list, &thread_current->felem);
    lock_release(&file_lock);

    return new_fd->fd;
  }
}

int sys_filesize (int fd)
{
  struct file_desc* open_file;

  lock_acquire(&file_lock);
  open_file = get_file_desc(fd);
  lock_release(&file_lock);
  return file_length(open_file->file);
}

int sys_read (int fd, void *buffer, unsigned size)
{
  int key;
  int i;
  int read_size;
  struct file_desc* open_file;

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
    if(open_file == NULL)
    {
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
  file_seek(open_file->file, position);
  lock_release(&file_lock);
}

unsigned sys_tell (int fd)
{
  unsigned next_pos;
  struct file_desc* open_file;

  lock_acquire(&file_lock);
  open_file = get_file_desc(fd);
  next_pos = file_tell(open_file->file);
  lock_release(&file_lock);

  return next_pos;
}

void sys_close (int fd)
{
  struct file_desc* open_file;

  lock_acquire(&file_lock);
  open_file = get_file_desc(fd);
  file_close(open_file->file);
  file_allow_write(open_file->file);
  remove_file_desc(fd);
  lock_release(&file_lock);
}

struct *file_desc get_file_desc(int fd)
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