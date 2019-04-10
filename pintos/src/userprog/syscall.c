#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "filesys/filesys.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

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

  is_valid_ptr(f->esp);

  //int sc_num = *(int*)f->esp;
  printf ("syscall number : %d\n", *(int*)f->esp);

  switch(*(int*)f->esp)
  {
    case SYS_HALT:		// 0.  0
	halt();
	break;

    case SYS_EXIT:		// 1.  1
    {
	int status = *((int*)f->esp+1);

	exit(status);
	break;
    }
    case SYS_EXEC:		// 2.  1
	break;

    case SYS_WAIT:		// 3.  1
	break;

    case SYS_CREATE:		// 4.  2
    {
	char *file_ = (char*)(*((int*)f->esp+3));
	unsigned initial_size = *((unsigned*)f->esp+4);

	f->eax = create(file_, initial_size);
	break;
    }
    case SYS_REMOVE:		// 5.  1
	break;

    case SYS_OPEN:		// 6.  1
    {
	char *file_ = (char*)(*((int*)f->esp+1));

	f->eax = open(file_);
	break;
    }
    case SYS_FILESIZE:		// 7.  1
	break;

    case SYS_READ:		// 8.  3
    {printf("************SYS_READ*************\n");
	int fd = *((int*)f->esp+5);
	void *buffer = (void*)(*((int*)f->esp+6));
	unsigned size = *((unsigned*)f->esp+7);

	f->eax = read(fd, buffer, size);
	break;
    }
    case SYS_WRITE:		// 9. 3
    {printf("************SYS_WRITE************\n");
	int fd = *((int*)f->esp+5);
	const void *buffer = (const void*)(*((int*)f->esp+6));
	unsigned size = *((unsigned*)f->esp+7);

	f->eax = write(fd, buffer, size);
	break;
    }
    case SYS_SEEK:		// 10. 2
	break;

    case SYS_TELL:		// 11. 1
	break;

    case SYS_CLOSE:		// 12. 1
	break;
  }

  //thread_exit ();
}

void halt (void)
{
  // Terminates Pintos
  shutdown_power_off();
}

void exit (int status)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct fd_file *ff;

  // Close all files in file_list of thread
  while(!list_empty(&t->file_list))
  {
    e = list_front(&t->file_list);
    ff = list_entry(e, struct fd_file, elem);

    file_close(ff->file_);
    list_remove(e);
  }  

  // Returning status??
  printf("<< %s >> exit with status << %d >>!!\n", t->name, status); 

  // process_exit() frees all resources. But will entire process be off..?
  process_exit();
}

// Just using function in filesys, create file.
bool create (const char *file, unsigned initial_size)
{
  return filesys_create(file, initial_size);
}

// It is different concept from making file. After open the file,
// opened file will get the fd(index in file list in thread).
// I make my own function, add_filelist();
int open (const char *file)
{
  struct file *f;
  f = filesys_open(file);

  if(f==NULL)
  {
    printf("Fail to open the file.\n");
    return -1;
  }

  else
  {
    printf("Success to open the file.\n");
    int fd = add_filelist(f);
    return fd;
  }
}

// Find the file by using fd (find_file()), then read it
// to the buffer (file_read()).
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
    struct file *f = find_file(fd);
    return file_read(f, buffer, length);
  }
}

// Find the file by using fd (find_file()), then write buffer
// to that file.
int write (int fd, const void *buffer, unsigned length)
{
  if(fd==1) // writes to the console by using putbuf()
  {
    putbuf(buffer, length); // I don't know how to use putbuf
    return length;
  }
  else
  {
    struct file *f = find_file(fd);
    return file_write(f, buffer, length);
  }
}


// just add opened file to file list in thread.
// It returns fd of this file.
int add_filelist (struct file *f)
{
  struct fd_file *ff;
  fd_file_init(ff);

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

// Find corresponding file among file list through fd.
struct file *find_file (int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct fd_file *ff;

  while(!list_empty(&t->file_list))
  {
    e = list_front(&t->file_list);
    ff = list_entry(e, struct fd_file, elem);

    if(ff->fd == fd)
      return ff->file_;

    list_next(e);
  }
  return NULL;
}

void is_valid_ptr (const void *ptr)
{
  //Is it null pointer?
  if(ptr==NULL)
    exit(-1);

  //Is it unmapped pointer?
  if(pagedir_get_page(thread_current()->pagedir, ptr)==NULL)
    exit(-1);

  //Is it pointing to kernel virtual memory?
  if(is_kernel_vaddr(ptr))
    exit(-1);
}

// Initializes fd_file as an empty structure
void fd_file_init (struct fd_file *fd_file)
{
  ASSERT (fd_file != NULL);
  fd_file->fd = 0;
  fd_file->elem.prev = NULL;
  fd_file->elem.next = NULL;
}


