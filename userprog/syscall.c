#include "userprog/syscall.h"
#include <stdio.h>
#include "threads/malloc.h"
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include "filesys/filesys.h"
#include "filesys/file.h"

#include "process.h"
#include "pagedir.h"

#include "threads/synch.h"

static struct semaphore join_sema[TH_NO];
static struct uthread_args process_uthreads_args[TH_NO];
static tid_t process_uthreads_tids[TH_NO];
static int process_uthread_status[TH_NO];

static void syscall_handler (struct intr_frame *);
void verify_addresses(void*, int);
void unexpected_exit();
static crt_thread_id = 0;

void
unexpected_exit()
{
  // should set current_thread exit status to -1
  thread_exit();
}

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
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int* addr = f->esp;

  struct file* file;
  char* fileName;
  off_t size;
  struct opened_file* op_f;
  int fd;
  char* buffer;

  //int syscall_no = ((int*)f->esp)[0];
	int no;
	char *buf;
	int waited_tid;
	int *status, th_status, i;


  verify_addresses(addr, 1);

	int syscall_no = addr[0];
	addr++;
  printf("\nSYSCALL %d\n", syscall_no);

	switch (syscall_no) {
    case SYS_HALT:
      shutdown_power_off();
      break;
    case SYS_WAIT:
      verify_addresses(addr, 1);
      f->eax = process_wait(*addr);
      break;
    case SYS_EXEC:
      verify_addresses(addr, 1);
      f->eax = process_execute((char)*addr);
      break;
		case SYS_EXIT:
      verify_addresses(addr, 1);
			//printf ("SYS_EXIT system call!\n");
      acquire_file_lock(); // maybe use lock internally for each file?
      close_all_files();
      release_file_lock();

      thread_current()->exit_status = addr[0];
      printf("%s: exit(%d)\n", thread_current()->name, thread_current()->exit_status);
      file = thread_current()->self;
			thread_exit();
      file_allow_write(file); // allow writing to executable
			break;

    case SYS_CREATE: // creates a new file, returns true/false, depending on the outcome
      verify_addresses(addr, 2);
      acquire_file_lock();
        fileName = (char*)addr[0];
        size = (int)addr[1];
        f->eax = filesys_create(fileName, size);
      release_file_lock();
      break;

    case SYS_REMOVE: // deletes file, returns true/false, depending on the outcome
      verify_addresses(addr, 1);
      acquire_file_lock();
        fileName = (char*)addr[0];
        f->eax = filesys_remove(fileName);
      release_file_lock();
      break;

    case SYS_OPEN: //opens file and returns its respective file descriptor
      verify_addresses(addr, 1);
      acquire_file_lock();
        fileName = (char*)addr[0];
        file = filesys_open(fileName);
      release_file_lock();
      if(file)
      {
        op_f = (struct opened_file*)malloc(sizeof(struct opened_file));
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
      fd = (int)addr[0];
      op_f = getFile(fd);
      if(op_f)
      {
        acquire_file_lock();
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
      fd = (int)addr[0];
      buffer = (char*)addr[1];
      size = (int)addr[2];
      if(fd == 0) // fd points to STDIN
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
          acquire_file_lock();
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
      fd = (int)addr[0];
      buffer = (char*)addr[1];
      size = (int)addr[2];
      if(fd == 1) // fd points to STDOUT
      {
        putbuf(buffer, size);
        f->eax = size;
      }
      else 
      {
        op_f = getFile(fd);
        if(op_f)
        {
          acquire_file_lock();
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
      fd = addr[0];
      int position = addr[1];
      op_f = getFile(fd);
      if(op_f)
      {
        acquire_file_lock();
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
      fd = addr[0];
      op_f = getFile(fd);
      if(op_f)
      {
        acquire_file_lock();
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
      fd = addr[0];
      op_f = getFile(fd);
      if(op_f)
      {
        acquire_file_lock();
          file_close(op_f->file);
        release_file_lock();
        list_remove(&op_f->file_elem);
        free(op_f->file);
        f->eax = 0;
      }
      else
      {
        f->eax = -1;
      }
      break;

      case SYS_UTHREAD_CREATE:
			printf ("SYS_UTHREAD_CREATE system call from thread %d!\n", thread_current()->tid);

			process_uthreads_args[crt_thread_id].th_fc_addr = ((int*)f->esp)[1];
			process_uthreads_args[crt_thread_id].th_fc_arg = ((int*)f->esp)[2];

		  snprintf(process_uthreads_args[crt_thread_id].th_name,
					sizeof(process_uthreads_args[crt_thread_id].th_name),
					"th-%d", crt_thread_id);

		process_uthreads_args[crt_thread_id].th_pid = thread_current()->pid;

			process_uthreads_args[crt_thread_id].th_pagedir = thread_current()->pagedir;

			
			process_uthreads_args[crt_thread_id].th_no = crt_thread_id;

			process_uthreads_tids[crt_thread_id] = process_uthread_execute(&process_uthreads_args[crt_thread_id]);

			if (process_uthreads_tids[crt_thread_id] != -1) {
				sema_init(&join_sema[crt_thread_id],0);

				f->eax = crt_thread_id;
			} else {
				f->eax = -1;
			}

			crt_thread_id++;
			return;
		case SYS_UTHREAD_JOIN:
			printf ("SYS_UTHREAD_JOIN system call from thread %d!\n", thread_current()->tid);

			waited_tid = ((int*)f->esp)[1];
			status = (int*)((int*)f->esp)[2];

			if (process_uthreads_tids[waited_tid] != -1) {
				
				sema_down(&join_sema[waited_tid]);

				*status = process_uthread_status[waited_tid];
				f->eax = 0;
			} else
				/* Return error - non existent thread */
				f->eax = -1;

			return;
		case SYS_UTHREAD_EXIT:
			printf ("SYS_UTHREAD_EXIT system call from thread %d!\n", thread_current()->tid);
			th_status = ((int*)f->esp)[1];

			process_uthread_status[thread_current()-> uthread_id] = th_status;

			for(int i = 0; i <TH_NO; i++){
				sema_up(&join_sema[thread_current()->uthread_id]);
			}

			// Terminate the thread
			thread_exit();

			f->eax=0;
			return;
		case SYS_UTHREAD_GETPID:
			printf ("SYS_UTHREAD_GETPID system call from thread %d!\n", thread_current()->tid);

			f->eax = thread_current()->pid;

			return;
		case SYS_UTHREAD_GETTID:
			printf ("SYS_UTHREAD_GETTID system call from thread %d!\n", thread_current()->tid);

			f->eax = thread_current()->uthread_id;

			return;

    default:
      unexpected_exit();
	}

	return;
}
