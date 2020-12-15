#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

struct uthread_args {
	char th_name[16];	// the name of the thread
	int th_pid;			// the pid the thread belongs to
	void* th_fc_addr;	// user function address
	int th_fc_arg;		// user function argument
	int th_no;			// current thread number
	uint32_t *th_pagedir;	// page directory of the process
};

tid_t process_uthread_execute (struct uthread_args *th_arg);



#endif /* userprog/process.h */
