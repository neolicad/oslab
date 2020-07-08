#define __LIBRARY__

#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define PAGE_SIZE 4096

_syscall3(int,shmget,key_t,key,size_t,size,int,shmflg)
_syscall3(void*,kshmat,int,shmid,const void*,shmaddr,int,shmflg)

void *shmat(int shmid, const void *shmaddr, int shmflg) {
  unsigned long ptr = (unsigned long) kshmat(shmid, shmaddr, shmflg);    
  /* 
   * This is a hack: brk here is not a simple system call. The implmentation 
   * in bochs vm's libc.a stores a state indicating the current brk, malloc 
   * then uses that state to decide where to start the 4-page space it 
   * manages. So even if we have set current->brk in kshmat, malloc will not 
   * use that current->brk as the start address for creating its managed space, 
   * i.e. If we don't call brk(ptr + PAGE_SIZE) here, malloc will have its 
   * managed space start from ptr, although it won't create a new page to 
   * override the physical memory ptr points to, it will wipe out the data in 
   * the physical page, which causes data loss. 
   */
  brk(ptr + PAGE_SIZE);
  return (void *) ptr;
}

