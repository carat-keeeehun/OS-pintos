#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");

  int sc_num = *(int*)f->esp;
  printf ("syscall number : %d\n", sc_num);

  switch(sc_num)
  {
    case SYS_HALT:		// 1.  0
	halt();
	break;

    case SYS_EXIT:		// 2.  1
	break;

    case SYS_EXEC:		// 3.  1
	break;

    case SYS_WAIT:		// 4.  1
	break;

    case SYS_CREATE:		// 5.  2
    {
	char *file_ = (char*)(*((int*)f->esp+1));
	unsigned initial_size = *((unsigned*)f->esp+2);

	f->eax = create(file_, initial_size);
	break;
    }
    case SYS_REMOVE:		// 6.  1
	break;

    case SYS_OPEN:		// 7.  1
    {
	char *file_ = (char*)(*((int*)f->esp+1));

	f->eax = open(file_);
	break;
    }
    case SYS_FILESIZE:		// 8.  1
	break;

    case SYS_READ:		// 9.  3
/*	int fd = *((int*)f->esp+1);
	void *buffer = (void*)(*((int*)f->esp+2));
	unsigned size = *((unsigned*)f->esp+3);

	f->eax = read(fd, buffer, size);*/
	break;

    case SYS_WRITE:		// 10. 3
	break;

    case SYS_SEEK:		// 11. 2
	break;

    case SYS_TELL:		// 12. 1
	break;

    case SYS_CLOSE:		// 13. 1
	break;
  }

  thread_exit ();
}

void halt (void)
{
  //Terminates Pintos
  shutdown_power_off();
}

bool create (const char *file, unsigned initial_size)
{
  return filesys_create(file, initial_size);
}

int open (const char *file)
{
  struct file *f;
  int fd;
  f = filesys_open(file);

  if(f==NULL)
  {
    printf("Fail to open the file.\n");
    return -1;
  }

  else
  {
    printf("Success to open the file.\n");
    int fd = add_filelist(file);
    return fd;
  }
}
/*
int read (int fd, void *buffer, unsigned length)
{
  if(fd==0) // reads from the keyboard using input_getc()
  {
    while(length > 0)
    {
      *(char*)buffer = input_getc();
      buffer++;
      length--;
    }
    return length;
  }
  else
  {
    return file_read(, buffer, length);
  }
}
*/
int add_filelist (struct file *f)
{
  struct fd_file *ff;
  struct thread *t = thread_current();
  
  if(list_begin(&t->file_list)==list_end(&t->file_list))
    t->f_num = 1;
  else
    t->f_num++;

    ff->fd = t->f_num;
    ff->file_ = f;
    // fd is increased-order, so I must use push_back
    list_push_back(&t->file_list, &ff->elem);
    return ff->fd;
}
