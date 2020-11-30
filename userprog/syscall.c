#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include "filesys/filesys.h"
#include "filesys/file.h"

#include "process.h"
#include "pagedir.h"

static void syscall_handler (struct intr_frame *);

/* 
  verifies a number of addresses used in system calls 
  launches an unexpected exit if address is bust
*/
void
verify_addresses(void *p, int num)
{
  void *iterator = p;
  for (int i = 0; i < num; i++, iterator++)
  {
    if( iterator == NULL ||
       !is_user_vaddr(iterator) ||
       !pagedir_get_page(thread_current()->pagedir, iterator))
    {
      unexpected_exit();
    }
  }
}

void
unexpected_exit()
{
  // should set current_thread exit status to -1
  thread_exit();
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int* addr = f->esp;
  verify_addresses(addr, 1);

	int syscall_no = (int*)addr++;
	printf ("system call no %d!\n", syscall_no);

	switch (syscall_no) {
		case SYS_EXIT:
			//printf ("SYS_EXIT system call!\n");
      aquire_file_lock(); // maybe use lock internally for each file?
        close_all_files();
      release_file_lock();
			thread_exit();
			break;

    case SYS_CREATE: // creates a new file, returns true/false, depending on the outcome
      verify_addresses(addr, 2);
      aquire_file_lock();
        const char* fileName = (char*)addr[0];
        off_t size = (int*)addr[1];
        f->eax = filesys_create(fileName, size);
      release_file_lock();
      break;

    case SYS_REMOVE: // deletes file, returns true/false, depending on the outcome
      verify_addresses(addr, 1);
      aquire_file_lock();
        const char* fileName = (char*)addr[0];
        f->eax = filesys_remove(fileName);
      release_file_lock();
      break;

    case SYS_OPEN: //opens file and returns its respective file descriptor
      verify_addresses(addr, 1);
      aquire_file_lock();
        const char* fileName = (char*)addr[0];
        struct file* file = filesys_open(fileName);
      release_file_lock();
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
      verify_addresses(addr, 1);
      int fd = addr[0];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        aquire_file_lock();
          f->eax = file_length(op_f->file);
        release_file_lock();
      }
      else 
      {
        f->eax = -1;
      }
      break;

    case SYS_READ: //reads from file into buffer, returns number of bytes actually read
      verify_addresses(addr, 3);
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
          aquire_file_lock();
            f->eax = file_read(op_f->file, buffer, size);
          release_file_lock();
        }
        else
        {
          f->eax = -1;
        }
      }
      break;

    case SYS_WRITE: //writes to file from buffer, returns number of bytess actualy written
      verify_addresses(addr, 3);
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
          aquire_file_lock();
            f->eax = file_write(op_f->file, buffer, size);
          release_file_lock();
        }
        else
        {
          f->eax = -1;
        }
      }
      break;

    case SYS_SEEK: //changes next byte to be accessed in the open file
      verify_addresses(addr, 2);
      int fd = addr[0];
      int position = addr[1];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        aquire_file_lock();
          file_seek(op_f->file, position);
        release_file_lock();
        f->eax = 0;
      }
      else
      {
        f->eax = -1;
      }
      break;

    case SYS_TELL: //returns the position of the next byte to be accessed from the file
      verify_addresses(addr, 1);
      int fd = addr[0];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        aquire_file_lock();
          f->eax = file_tell(op_f->file);
        release_file_lock();
      }
      else
      {
        f->eax = -1;
      }
      break;

    case SYS_CLOSE: //closes file descriptor
      verify_addresses(addr, 1);
      int fd = addr[0];
      struct opened_file* op_f = getFile(fd);
      if(op_f)
      {
        aquire_file_lock();
          file_close(op_f->file);
        release_file_lock();
        list_remove(&op_f->file_elem);
        free(op_f);
        f->eax = 0;
      }
      else
      {
        f->eax = -1;
      }
      break;

    default:
      unexpected_exit();
	}

	thread_exit ();
}
