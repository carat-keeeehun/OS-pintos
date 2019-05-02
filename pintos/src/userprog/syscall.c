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
  //printf ("system call!\n");
  is_valid_ptr(f->esp);

  //int sc_num = *(int*)f->esp;
//  printf ("#####   syscall number : %d\n", *(int*)f->esp);
//  printf ("#####   [%s] status : %d\n", thread_current()->name, thread_current()->status);
  switch(*(int*)f->esp)
  {
    case SYS_HALT:		// 0.  0
	halt();
	break;

    case SYS_EXIT:		// 1.  1
    {
	is_valid_ptr(f->esp+1);
	int status = *((int*)f->esp+1);
//	printf("***********SYS_EXIT***********\n");
	exit(status);
//printf("finish of exit function\n");
	break;
    }
    case SYS_EXEC:		// 2.  1
    {
	is_valid_ptr(f->esp+1);
	char *cmd_line = (char*)(*((int*)f->esp+1));
//	printf("***********SYS_EXEC***********\n");
//	printf("cmd_line : %s\n", cmd_line);

	f->eax = exec(cmd_line);
	break;
    }
    case SYS_WAIT:		// 3.  1
    {
	is_valid_ptr(f->esp+1);
	pid_t pid = (pid_t*)(*((int*)f->esp+1));
//	printf("***********SYS_WAIT***********\n");
	wait(pid);

	break;
    }
    case SYS_CREATE:		// 4.  2
    {
	is_valid_ptr(f->esp+4);
	is_valid_ptr(f->esp+5);
	char *file_ = (char*)(*((int*)f->esp+4));
	unsigned initial_size = *((unsigned*)f->esp+5);
//printf("***********SYS_CREATE***********\n");
//printf("file : %s\n", *file_);
//printf("initial_size : %d\n", initial_size);
	f->eax = create(file_, initial_size);
	break;
    }
    case SYS_REMOVE:		// 5.  1
    {
	//char *file_ = (char*)(*((int*)f->esp+1));
//	printf("***********SYS_REMOVE***********\n");

	break;
    }
    case SYS_OPEN:		// 6.  1
    {
	is_valid_ptr(f->esp+1);
	char *file_ = (char*)(*((int*)f->esp+1));

	f->eax = open(file_);
	break;
    }
    case SYS_FILESIZE:		// 7.  1
    {
	//int fd = *((int*)f->esp+1);
//	printf("***********SYS_FILESIZE***********\n");

	break;
    }
    case SYS_READ:		// 8.  3
    {//printf("************SYS_READ*************\n");
	is_valid_ptr(f->esp+5);
	is_valid_ptr(f->esp+6);
	is_valid_ptr(f->esp+7);
	int fd = *((int*)f->esp+5);
	void *buffer = (void*)(*((int*)f->esp+6));
	unsigned size = *((unsigned*)f->esp+7);

	f->eax = read(fd, buffer, size);
	break;
    }
    case SYS_WRITE:		// 9. 3
    {//printf("************SYS_WRITE************\n");
	is_valid_ptr(f->esp+5);
	is_valid_ptr(f->esp+6);
	is_valid_ptr(f->esp+7);
	int fd = *((int*)f->esp+5);
	const void *buffer = (const void*)(*((int*)f->esp+6));
	unsigned size = *((unsigned*)f->esp+7);

	f->eax = write(fd, buffer, size);
	break;
    }
    case SYS_SEEK:		// 10. 2
    {
	//int fd = *((int*)f->esp+5);
	//unsigned position = *((unsigned*)f->esp+7);
//	printf("***********SYS_SEEK***********\n");

	break;
    }
    case SYS_TELL:		// 11. 1
    {
	//int fd = *((int*)f->esp+1);
//	printf("***********SYS_TELL***********\n");
	break;
    }
    case SYS_CLOSE:		// 12. 1
    {
	//int fd = *((int*)f->esp+1);
//	printf("***********SYS_CLOSE***********\n");
	break;
    }
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
  struct thread *ct;
  struct list_elem *e;
  struct fd_file *ff;
  
  // Close all files in file_list of thread
//printf("[%s] f_num = %d\n", t->name, t->f_num);
  if(t->f_num != 0)
  {
    while(!list_empty(&t->file_list))
    {
      e = list_front(&t->file_list);
      ff = list_entry(e, struct fd_file, elem);
      file_close(ff->file_);
      list_remove(e);
    }
  }

  // Returning status??
  //printf("'%s' exit with status [%d]\n", t->name, t->status);
//printf("------ In exit function ------\n");
//printf("       Before removed, child's parent : %s\n",  t->parent->name);
  // Remove itself as parent, so it makes its children orphan.
//printf("[%s] c_num = %d\n", t->name, t->c_num);

  // It is removed in child_list of parent.
  if(t->parent != NULL)
  {
    //printf("       It has parent[%s]\n", t->parent->name);
    t->exit_status = status;
    t->parent->c_num--;
    list_remove(&t->c_elem);
  }
  else
    t->exit_status = status;
  //printf("       It has no parent.\n");

printf("%s: exit(%d)\n", t->name, t->exit_status);
  // Frees all resources && Remove this child_thread.
  thread_exit();
}

pid_t exec (const char *cmd_line)
{
  //printf("In exec syscall\n");
  return process_execute(cmd_line);
}

int wait (pid_t pid)
{
  return process_wait(pid);
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
  struct file *f = filesys_open(file);
//if(list_empty(&thread_current()->file_list))
//printf("In open, still empty\n");
  if(f==NULL)
  {
//    printf("Fail to open the file.\n");
    return -1;
  }
  else
  {
//    printf("Success to open the file.\n");
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
  struct fd_file *ff = malloc(sizeof(*ff));
  struct thread *t = thread_current();

  t->f_num++;
  ff->fd = t->f_num;
  ff->file_ = f;
  
  // fd is increased-order, so I must use push_back
  list_push_back(&t->file_list, &ff->elem);
//printf("ff->fd : %d\n", ff->fd);
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

