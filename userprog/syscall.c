#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include "filesys/filesys.h"
#include "filesys/file.h"

#include "process.h"

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
      // verify 1 address
      // aquire file lock
      const char* fileName = (char*)addr[0];
      struct file* file = filesys_open(fileName);
      // release file lock
      if(file)
      {
        struct opened_file* op_f = malloc(sizeof(struct opened_file));
        op_f->fd = fd_counter++;
        op_f->file = file;
        list_push_back(&files, &op_f->file_elem); // push file to process list
        
        f->eax = op_f->fd; // return file descriptor
      }
      else
      {
        f->eax = -1; // opening failed
      }
      break;

    case SYS_FILESIZE: //returns size of file
      // verify 1 address
      int fd = addr[0];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        // aquire file lock
        f->eax = file_length(op_f->file);
        // release file lock
      }
      else 
      {
        f->eax = -1;
      }
      break;

    case SYS_READ: //reads from file into buffer, returns number of bytes actually read
      // verify 3 addresses
      int fd = addr[0];
      int* buffer = addr[1];
      int size = addr[2];
      if(fd == 0) // fd points to STDOUT
      {
        for (int i = 0; i < size; i++)
        {
          buffer[i] = input_getc();
        }
        f->eax = size;
      }
      else 
      {
        struct opened_file* op_f = getFile(fd);
        if(op_f)
        {
          // aquire file lock
          f->eax = file_read(op_f->file, buffer, size);
          // release file lock
        }
        else
        {
          f->eax = -1;
        }
      }
      break;

    case SYS_WRITE: //writes to file from buffer, returns number of bytess actualy written
      // verify 3 addresses
      int fd = addr[0];
      int* buffer = addr[1];
      int size = addr[2];
      if(fd == 1) // fd points to STDIN
      {
        putbuf(buffer, size);
        f->eax = size;
      }
      else 
      {
        struct opened_file* op_f = getFile(fd);
        if(op_f)
        {
          // aquire file lock
          f->eax = file_write(op_f->file, buffer, size);
          // release file lock
        }
        else
        {
          f->eax = -1;
        }
      }
      break;

    case SYS_SEEK: //changes next byte to be accessed in the open file
      // verify 2 addresses
      int fd = addr[0];
      int position = addr[1];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        // aquire file lock
        file_seek(op_f->file, position);
        // release file lock
        f->eax = 0;
      }
      else
      {
        f->eax = -1;
      }
      break;

    case SYS_TELL: //returns the position of the next byte to be accessed from the file
      // verify 1 address
      int fd = addr[0];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        // aquire file lock
        f->eax = file_tell(op_f->file);
        // release file lock
      }
      else
      {
        f->eax = -1;
      }
      break;

    case SYS_CLOSE: //closes file descriptor
      // verify 1 address
      int fd = addr[0];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        // aquire file lock
        file_close(op_f->file);
        // release file lock
        list_remove(&op_f->file_elem);
        free(op_f);
        f->eax = 0;
      }
      else
      {
        f->eax = -1;
      }
      break;
	}

	thread_exit ();
}
