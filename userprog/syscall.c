#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int* addr = f->esp;
  //verify pointer

	int syscall_no = (int*)addr++;
	printf ("system call no %d!\n", syscall_no);

	switch (syscall_no) {
		case SYS_EXIT:
			//printf ("SYS_EXIT system call!\n");
			thread_exit();
			break;

    case SYS_CREATE: // creates a new file, returns true/false, depending on the outcome
      // verify 2 addresses
      // aquire file lock
      const char* fileName = (char*)addr[0];
      off_t size = (int*)addr[1];
      f->eax = filesys_create(fileName, size);
      // release file lock
      break;

    case SYS_REMOVE: // deletes file, returns true/false, depending on the outcome
      // verify 1 address
      // aquire file lock
      const char* fileName = (char*)addr[0];
      f->eax = filesys_remove(fileName);
      // release file lock
      break;

    case SYS_OPEN: //opens file and returns its respective file descriptor
      return false;

    case SYS_FILESIZE: //returns size of file
      return 0;

    case SYS_READ: //reads from file into buffer, returns number of bytes actually read
      return 0;

    case SYS_WRITE: //writes to file from buffer, returns number of bytess actualy written

			int fd = ((int*)f->esp)[1];
			int buf = (char*) ((int*)f->esp)[2];
			int num = ((int*)f->esp)[3];

			if (fd == 1)
				putbuf(buf, num);

			f->eax = num;
			return;
      return 0;

    case SYS_SEEK: //changes next byte to be accessed in the open file
      break;

    case SYS_TELL: //returns the position of the next byte to be accessed from the file
      return 0;

    case SYS_CLOSE: //closes file descriptor
      break;

	}

	thread_exit ();
}
