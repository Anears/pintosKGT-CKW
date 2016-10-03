#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/filesys.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);
void check_address(void *address);
void get_argument(int32_t *esp, int32_t *args, int count);
void halt(void);
void exit(int status);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  void *esp=f->esp;
  check_address(esp);
  int syscall_num=*(int *)esp;
  int args[5];

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
      f->eax=create((const char *)args[0],args[1]);
      break;
    case SYS_REMOVE:
      get_argument(esp,args,1);
      f->eax=remove((const char *)args[0]);
      break;
   /* case SYS_WRITE:
      get_argument(esp,arg,3);
      check_adress(arg[1]);
      f->eax=write(arg[0],arg[1],arg[2]);
      break;*/
    default:
      exit(-1);
  }
  //printf ("system call!\n");
  //thread_exit ();
}

void
check_address(void *address)
{
  if(address<=(void *)0x8048000 || address>=(void *)0xc0000000)
    exit(-1);
}

void
get_argument(int32_t *esp, int32_t *args, int count)
{
  while(count--){
    check_address(++esp);
    *(args++)=*esp;
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
  struct thread *tc=thread_current();
  printf("%s: exit(%d)\n",tc->name,status);
  tc->exit_status=status;
  thread_exit();
}

bool
create(const char *file, unsigned initial_size)
{
  return filesys_create(file,initial_size);
}

bool
remove(const char *file)
{ 
  return filesys_remove(file);
}
