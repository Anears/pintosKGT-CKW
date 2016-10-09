#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <filesys/file.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/filesys.h"
#include "devices/shutdown.h"
#include <devices/input.h>
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);
void check_address(void *address);
void get_argument(unsigned int *esp, int *args, int count);
void halt(void);
void exit(int status);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd,void *buffer,unsigned size);
int write(int fd,void *buffer,unsigned size);
void seek(int fd,unsigned position);
unsigned tell(int fd);
void close(int fd);
tid_t exec(const char*cmd_line);
int wait(tid_t tid);

void
syscall_init (void) 
{
  lock_init(&sys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  unsigned int *esp=(unsigned int *)(f->esp);
  check_address((void *)esp);
  int syscall_num=*(int *)esp;
  int args[5];

  esp++;
  check_address(esp);
  switch(syscall_num)
  {
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      get_argument(esp,args,1);
      exit(args[0]);
      break;
    case SYS_CREATE:
      get_argument(esp,args,2);
      f->eax=create((const char *)args[0],(unsigned)args[1]);
      break;
    case SYS_REMOVE:
      get_argument(esp,args,1);
      f->eax=remove((const char *)args[0]);
      break;
    case SYS_OPEN:
      get_argument(esp,args,1);
      f->eax=open((const char *)args[0]);
      break;
    case SYS_FILESIZE:
      get_argument(esp,args,1);
      f->eax=filesize(args[0]);
      break;
    case SYS_READ:
      get_argument(esp,args,3);
      f->eax=read(args[0],(void *)args[1],(unsigned)args[2]);
      break;
    case SYS_WRITE:
      get_argument(esp,args,3);
      f->eax=write(args[0],(void *)args[1],(unsigned)args[2]);
      break;
    case SYS_SEEK:
      get_argument(esp,args,2);
      seek(args[0],(unsigned)args[1]);
      break;
    case SYS_TELL:
      get_argument(esp,args,1);
      f->eax=tell(args[0]);
      break;
    case SYS_CLOSE:
      get_argument(esp,args,1);
      close(args[0]);
      break;
    case SYS_EXEC:
      get_argument(esp,args,1);
      f->eax=exec((const char *)args[0]);
      break;
    case SYS_WAIT:
      get_argument(esp,args,1);
      f->eax=wait(args[0]);
      break;
    default:
      //printf("\n\n으앙으앙으앙으앙으앙\n\n");
      exit(-1);
  }
  //printf ("system call!\n");
  //thread_exit ();
}

void
check_address(void *address)
{
  if((unsigned int )address<=0x8048000 || (unsigned int)address>=0xc0000000){
    //printf("\n\nsyscall.c check_address call exit\n\n");
    exit(-1);
  }
}

void
get_argument(unsigned int *esp, int *args, int count)
{
 int i;
  for(i=0;i<count;++i){
    check_address((void *)esp);
    args[i]=(int)*(esp);
    esp++;
  }
}

void
halt(void)
{
  shutdown_power_off();
}

void
exit(int status)
{
  struct thread *t=thread_current();
  printf("%s: exit(%d)\n",t->name,status);
  t->exit_status=status;
  //printf("!@!@!@!@!@!@!@!@!@!@!@!@");
  thread_exit();
}

int
wait(tid_t tid)
{
  return process_wait(tid);
}

bool
create(const char *file, unsigned initial_size)
{
  check_address((void *)file);
  return filesys_create(file,initial_size);
}

bool
remove(const char *file)
{
  check_address((void *)file);
  return filesys_remove(file);
}

int
open(const char *file)
{
  check_address((void *)file);
  lock_acquire(&sys_lock);
  struct file *fn=filesys_open(file);
  if(fn==NULL)
  {
    lock_release(&sys_lock);
    return -1;
  }
  int fd=process_add_file(fn);
  lock_release(&sys_lock);
  return fd;
}

int
filesize(int fd)
{
  struct file *f=process_get_file(fd);
  if(f==NULL) return -1;
  else return file_length(f);
}

int
read(int fd,void *buffer,unsigned size)
{
  struct file *f;
  check_address(buffer);
  lock_acquire(&sys_lock);
  if(fd==STDIN_FILENO)
  {
    unsigned i;
    for(i=0;i<size;++i)
      ((char *)buffer)[i]=input_getc();
    lock_release(&sys_lock);
    return size;
  }
  if((f=process_get_file(fd))==NULL)
  {
    lock_release(&sys_lock);
    return -1;
  }
  int fs=file_read(f,buffer,size);
  lock_release(&sys_lock);
  return fs;
}

int
write(int fd,void *buffer,unsigned size)
{
  struct file *f;
  check_address(buffer);
  lock_acquire(&sys_lock);
  if(fd==STDOUT_FILENO)
  {
    putbuf(buffer,size);
    lock_release(&sys_lock);
    return size;
  }
  if((f=process_get_file(fd))==NULL)
  {
    lock_release(&sys_lock);
    return -1;
  }
  int fs=file_write(f,buffer,size);
  lock_release(&sys_lock);
  return fs;
}

void
seek(int fd,unsigned position)
{
  struct file *f=process_get_file(fd);
  if(f==NULL) return;
  file_seek(f,(off_t)position);
}

unsigned
tell(int fd)
{
  struct file *f=process_get_file(fd);
  if(f==NULL) return -1;
  return file_tell(f);
}

void
close(int fd)
{
  process_close_file(fd);
}

tid_t
exec(const char *file)
{
  tid_t pid=process_execute(file);
  struct thread *child=get_child_process(pid);

  if(child)
  {
    sema_down(&child->load_sema);
    if(child->is_load) return pid;
    else return -1;
  }
  else return -1;
}
