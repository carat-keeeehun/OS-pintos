#include "userprog/syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "filesys/filesys.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
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
//	printf("exit argument 1 : %d\n", status);
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

	f->eax = wait(pid);
	break;
    }
    case SYS_CREATE:		// 4.  2
    {
	is_valid_ptr(f->esp+4);
	is_valid_ptr(f->esp+5);
	char *file_ = (char*)(*((int*)f->esp+4));
	unsigned initial_size = *((unsigned*)f->esp+5);
//printf("***********SYS_CREATE***********\n");
//printf("file : %s\n", file_);
//printf("create initial_size : %d\n", initial_size);

	lock_acquire(&fslock);
	f->eax = create(file_, initial_size);
	lock_release(&fslock);
	break;
    }
    case SYS_REMOVE:		// 5.  1
    {
	is_valid_ptr(f->esp+1);
	char *file_ = (char*)(*((int*)f->esp+1));
//	printf("***********SYS_REMOVE***********\n");

	lock_acquire(&fslock);
	f->eax = remove(file_);
	lock_release(&fslock);
	break;
    }
    case SYS_OPEN:		// 6.  1
    {//   printf("**************SYS_OPEN*************\n");
	is_valid_ptr(f->esp+1);
	char *file_ = (char*)(*((int*)f->esp+1));
//printf("file : %s\n", file_);

	lock_acquire(&fslock);
	f->eax = open(file_);
	lock_release(&fslock);
	break;
    }
    case SYS_FILESIZE:		// 7.  1
    {
	int fd = *((int*)f->esp+1);
//	printf("***********SYS_FILESIZE***********\n");

	lock_acquire(&fslock);
	f->eax = filesize(fd);
	lock_release(&fslock);
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

	lock_acquire(&fslock);
	f->eax = read(fd, buffer, size);
	lock_release(&fslock);
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

//printf("write argument 3-1 : %d\n", fd);
	lock_acquire(&fslock);
//printf("after lock\n");
	f->eax = write(fd, buffer, size);
	lock_release(&fslock);
	break;
    }
    case SYS_SEEK:		// 10. 2
    {
	int fd = *((int*)f->esp+5);
	unsigned position = *((unsigned*)f->esp+7);
//	printf("***********SYS_SEEK***********\n");

	lock_acquire(&fslock);
	seek(fd, position);
	lock_release(&fslock);
	break;
    }
    case SYS_TELL:		// 11. 1
    {
	int fd = *((int*)f->esp+1);
//	printf("***********SYS_TELL***********\n");

	lock_acquire(&fslock);
	f->eax = tell(fd);
	lock_release(&fslock);
	break;
    }
    case SYS_CLOSE:		// 12. 1
    {
	int fd = *((int*)f->esp+1);
//	printf("***********SYS_CLOSE***********\n");
//	printf("In close, fd = %d\n", fd);

	lock_acquire(&fslock);
	close(fd);
	lock_release(&fslock);
	break;
    }
  }

  //thread_exit ();
}

//          [0]
void halt (void)
{
  // Terminates Pintos
  shutdown_power_off();
}

//          [1]
void exit (int status)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  //int count = 0;

  // I wonder exact range of status.
  // Why -268370093 is bad arg?
  if(status < 0)
    status = -1;  
  
  // Close all files in file_list of thread
//printf("[%s] f_num = %d\n", t->name, t->f_num);
  if(t->f_num != 0)
  {//printf("Close all files in file list\n");
    for(e = list_begin(&t->file_list); e != list_end(&t->file_list);
	e = list_next(e))
    {
      struct fd_file *ff = list_entry(e, struct fd_file, elem);
      //printf("[%d] ff->fd = %d\n", count++, ff->fd);
      file_close(ff->file_);
      list_remove(e);
     // free(ff);
    }
  }

//  printf("[%s] c_num = %d\n", t->name, t->c_num);

//  if(t->c_num != 0)

  // Returning status??
  //printf("'%s' exit with status [%d]\n", t->name, t->status);
//printf("------ In exit function ------\n");
//printf("       Before removed, child's parent : %s\n",  t->parent->name);
  // Remove itself as parent, so it makes its children orphan.
//printf("[%s] c_num = %d\n", t->name, t->c_num);

  // It is removed in child_list of parent.
  if(t->parent != NULL)
  {
//    printf("       It has parent[%s]\n", t->parent->name);
    t->parent->child_exit_status = status;
    t->parent->c_num--;
    list_remove(&t->c_elem);
    printf("%s: exit(%d)\n", t->name, t->parent->child_exit_status);
  }
  else
  {
    printf("      It has no parent <<< orphan child case >>> \n");
    printf("%s: exit(-1)\n", t->name);
  }

  // Frees all resources && Remove this child_thread.
  thread_exit();
}

//          [2]
pid_t exec (const char *cmd_line)
{
 // printf("In exec syscall\n");
  char *exec_name;
  struct file *f;

  exec_name = malloc (strlen(cmd_line)+1);
  strlcpy(exec_name, cmd_line, strlen(cmd_line)+1);

  char *sptr;
  exec_name = strtok_r(exec_name, " ", &sptr);

  return process_execute(cmd_line);
}

//          [3]
int wait (pid_t pid)
{
  int result = process_wait(pid);
//  printf("In wait system call, return : %d\n", result);
  return result;
}

//          [4]
// Just using function in filesys, create file.
bool create (const char *file, unsigned initial_size)
{
  if(file == NULL)
    exit(-1);

  return filesys_create(file, initial_size);
}

//          [5]
bool remove (const char *file)
{
  bool result = filesys_remove(file);

  if(result == NULL) return false;
  else return true;
}

//          [6]
// It is different concept from making file. After open the file,
// opened file will get the fd(index in file list in thread).
// I make my own function, add_filelist();
int open (const char *file)
{
//  struct file *f = filesys_open(file);

  if(file == NULL)
  {
    return -1;
  }
  else
  {
    struct file *f = filesys_open(file);
    
    if(f == NULL)
    { //printf("Fail to filesys_open\n");
      return -1;
    }
    //printf("Success to open the file.\n");
    int fd = add_filelist(f);
//printf("In open, fd = %d\n", fd);
    return fd;
  }
}

//          [7]
int filesize (int fd)
{
  int filesize;

  struct fd_file *ff = find_file(fd);

  // Cannot find such file.
  if(ff == NULL)
    return -1;
  else
  {
    filesize = file_length(ff->file_);
    return filesize;
  }
}

//          [8]
// Find the file by using fd (find_file()), then read it
// to the buffer (file_read()).
int read (int fd, void *buffer, unsigned length)
{
//printf("In read, length : %d\n", length);
  if(fd==0) // reads from the keyboard using input_getc()
  {//printf("In 1read, fd = %d\n",fd);
    while(length > 0)
    {
      *(char*)buffer = input_getc();
      buffer++;
      length--;
    }
    return length;
  }
  else
  {//printf("In 2read, fd = %d\n", fd);
    struct fd_file *ff = find_file(fd);
//printf("After find_file, ff->fd : %d\n", ff->fd);
    if(ff == NULL)
      return -1;

    struct file *f = ff->file_;
//    int result = file_read(f, buffer, length);
//    printf("In read, return : %d\n", result);
    return file_read(f, buffer, length);
  }
}

//          [9]
// Find the file by using fd (find_file()), then write buffer
// to that file.
int write (int fd, const void *buffer, unsigned length)
{
//printf("In write, fd = %d\n", fd);
  if(fd==1) // writes to the console by using putbuf()
  {
    putbuf(buffer, length); // I don't know how to use putbuf
    return length;
  }
  else
  {
    struct fd_file *ff = find_file(fd);

    if(ff == NULL)
      return -1;

    struct file *f = ff->file_;
    return file_write(f, buffer, length);
  }
}

//          [10]
void seek (int fd, unsigned position)
{
  struct fd_file *ff = find_file(fd);

  if(ff == NULL);
    exit(-1);

  struct file *f = ff->file_;
  file_seek(f, position);
}

//          [11]
unsigned tell (int fd)
{
  struct fd_file *ff = find_file(fd);

  if(ff == NULL);
    exit(-1);

  struct file *f = ff->file_;
  return file_tell(f);
}


//          [12]
void close (int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;

  for(e = list_begin(&t->file_list); e != list_end(&t->file_list);
      e = list_next(e))
  {
    struct fd_file *ff = list_entry(e, struct fd_file, elem);
//    printf("((((  %d  ))))\n", count++);
    if(ff->fd == fd)
    {//printf("Find corresponding fd : %d\n", ff->fd);
      file_close(ff->file_);
      list_remove(e);
//printf("After remove\n");
      // Solve the problem in stuck certain test.
      free(ff);
      break;
    }
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

  for(e = list_begin(&t->file_list); e != list_end(&t->file_list);
      e = list_next(e))
  {
    struct fd_file *ff =list_entry(e, struct fd_file, elem);
    if(ff->fd == fd)
      return ff;
  }

  // In case of that cannot finding corresponding file
  return NULL;
}

void is_valid_ptr (const void *ptr)
{
  //Is it null pointer?
  if(ptr==NULL)
    exit(-1);

  //Is it pointing to kernel vertual memory?
  if(is_kernel_vaddr(ptr))
    exit(-1);

  //Is it unmapped pointer?
  if(pagedir_get_page(thread_current()->pagedir, ptr)==NULL)
    exit(-1);
}

