#include <unistd.h>

#define __LIBRARY__

typedef unsigned int key_t;

_syscall3(int,shmget,key_t,key,size_t,size,int,shmflg)
_syscall3(void*,shmat,int,shmid,const void*,shmaddr,int,shmflg)
