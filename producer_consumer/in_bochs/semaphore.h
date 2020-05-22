#define __LIBRARY__
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

typedef struct Sem {
  int id;
} sem_t;

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

