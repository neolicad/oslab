#define __LIBRARY__

#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

_syscall3(int,shmget,key_t,key,size_t,size,int,shmflg)
_syscall3(void*,shmat,int,shmid,const void*,shmaddr,int,shmflg)

