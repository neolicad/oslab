#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
