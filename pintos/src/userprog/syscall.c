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
  printf ("AAAAAAAAAAAA : %p\n", f->esp);

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
	break;
    case SYS_REMOVE:		// 6.  1
	break;
    case SYS_OPEN:		// 7.  1
	break;
    case SYS_FILESIZE:		// 8.  1
	break;
    case SYS_READ:		// 9.  3
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

void exit (int status)
{

}

 
