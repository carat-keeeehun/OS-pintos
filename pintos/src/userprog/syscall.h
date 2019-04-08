#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>

void syscall_init (void);

struct fd_file
{
  int fd;
  struct file *file_;
  struct list_elem elem; 
};
int add_filelist(struct file *f);

void halt (void);
//void exit (int status);
//pid_t exec (const char *file);
//int wait (pid_t);
bool create (const char *file, unsigned initial_size);
//bool remove (const char *file);
int open (const char *file);
//int filesize (int fd);
//int read (int fd, void *buffer, unsigned length);
//int write (int fd, const void *buffer, unsigned length);
//void seek (int fd, unsigned position);
//unsigned tell (int fd);
//void close (int fd);

#endif /* userprog/syscall.h */
