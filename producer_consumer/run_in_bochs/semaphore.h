#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

typedef struct Sem {
  int id;
} sem_t;

extern sem_t *sem_open(const char *name, unsigned int value); 
extern int sem_wait(sem_t *sem);
extern int sem_post(sem_t *sem);
extern int sem_unlink(const char *name);

#endif
