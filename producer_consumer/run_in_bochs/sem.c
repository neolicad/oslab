#define __LIBRARY__

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

_syscall2(int,sem_create,const char*,name,unsigned int,value)
_syscall1(int,sem_P,int,i)
_syscall1(int,sem_V,int,i)
_syscall1(int,sem_unlink,const char*,name)

sem_t *sem_open(const char *name, unsigned int value) {
  int id;
  sem_t *sem;
  if ((sem=(sem_t *) malloc(sizeof(sem_t))) == NULL) {
    return NULL;
  }
  if ((id=sem_create(name, value)) < 0) {
    return NULL;
  }
  sem->id = id;
  return sem;
}

int sem_wait(sem_t *sem) {
  if (sem == NULL) {
    return -1;
  }
  return sem_P(sem->id);
}

int sem_post(sem_t *sem) {
  if (sem == NULL) {
    return -1;
  }
  return sem_V(sem->id);
}

sem_t *create_sem(const char *name, int value) {
  sem_t *semaphore;
  if ((semaphore = sem_open(name, value)) == NULL) {
    printf(
        "%s at %d: failed to create semaphore: %s, error:%s\n", 
        __FILE__,
        __LINE__,
        name, 
        strerror(errno));
    exit(1);
  }
  return semaphore;
}
