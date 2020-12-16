#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
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
  int *p = f->esp;
  int syscall_nr = *p++;
  switch(syscall_nr) {
    case SYS_HALT:
      shutdown_power_off();
      break;
    case SYS_EXIT:
      thread_current()->exit_status = *p;
      thread_exit();
      break;
    case SYS_WAIT:
      f->eax = process_wait(*p);
      break;
    case SYS_EXEC:
      f->eax = process_execute((char)*p);
  }
  // printf ("system call!\n");
  // thread_exit ();
}
