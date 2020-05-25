#ifndef _SYS_SHM_H
#define _SYS_SHM_H
#include <sys/types.h>

#ifndef _KEY_T
#define _KEY_T
typedef unsigned int key_t;
#endif

extern int shmget(key_t key, size_t size, int shmflg);
extern void *shmat(int shmid, const void *shmaddr, int shmflg);

#endif
