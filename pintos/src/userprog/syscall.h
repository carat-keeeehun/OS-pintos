#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdlib.h>
#include <list.h>
#include "lib/user/syscall.h"
#include "threads/interrupt.h"

void syscall_init (void);

void is_valid_ptr(const void *ptr);
struct file *find_file (int fd);

struct fd_file
{
  int fd;
  struct file *file_;
  struct list_elem elem; 
};

void fd_file_init(struct fd_file *ff);
int add_filelist(struct file *f);


void halt (void);
void exit (int status);
pid_t exec (const char *file);
int wait (pid_t pid);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

#endif /* userprog/syscall.h */
