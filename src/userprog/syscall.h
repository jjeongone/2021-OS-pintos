#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

struct file_desc
{
   int fd;
   struct file *file;
};

void syscall_init (void);
void check_address (void *addr);
static int get_user (const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byte);
void get_stack_argument (void *esp, int byte_size, char *argument);

void sys_halt();
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

#endif /* userprog/syscall.h */
