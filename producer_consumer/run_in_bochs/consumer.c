#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "pc.h"

#define N 10

sem_t *Empty; 
sem_t *Full; 
sem_t *Mutex; 
vendor_t *Vendor;

void Consumer();
extern sem_t *create_sem(const char *name, int value);

int main() {
  int i;
  pid_t wpid;
  int status = 0;
  int shm_id;

  shm_id = shmget(KEY, sizeof(vendor_t), 0666); 
  if (shm_id == -1) {
    printf(
        "%s at %d: failed to create shared memory! error: %s\n", 
        __FILE__,
        __LINE__, 
        strerror(errno));
    exit(1);
  }
  Vendor = (vendor_t *) shmat(shm_id, NULL, 0);
  if ((int) Vendor == -1) {
    printf(
        "%s at %d: failed to attach shared memory to the local memory space! "
        "error: %s\n", 
        __FILE__,
        __LINE__, 
        strerror(errno));
    exit(1);
  }
  Empty = create_sem("empty", 10);
  Full = create_sem("full", 0);
  Mutex = create_sem("mutex", 1);
  for (i = 0; i < N; i++) {
    if (!fork()) {
       Consumer();
    }
  }        
  while ((wpid = wait(&status)) > 0);
  return 0;
}

void Consumer() {
  int num;
  int next_full;

  while (1) {
    sem_wait(Full);
    sem_wait(Mutex);
    next_full = Vendor->next_full;  
    num = Vendor->slots[next_full++];
    Vendor->next_full = next_full == SLOTS ? 0 : next_full;
    printf("%d: %d\n", getpid(), num);
    fflush(stdout);
    sem_post(Mutex);
    sem_post(Empty);
  }
}

