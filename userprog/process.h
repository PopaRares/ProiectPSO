#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

struct opened_file
{
    int fd;
    struct file* file;
    struct list_elem file_elem;
};

static int fd_counter;
extern struct list files;

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

struct opened_file* getFile(int);
void close_all_files(void);

#endif /* userprog/process.h */
