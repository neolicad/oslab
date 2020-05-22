#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vendor.h"

#define M 500

sem_t *Empty; 
sem_t *Full; 
sem_t *Mutex; 
vendor_t *Vendor;

void Producer();
extern sem_t *create_sem(const char *name, int value);

int main() {
  int shm_id;
  shm_id = shmget(ftok(".", 'x'), sizeof(vendor_t), IPC_CREAT | 0666); 
  if (shm_id == -1) {
    printf(
        "%d: failed to create shared memory! error: %s\n", 
        __LINE__, 
        strerror(errno));
    exit(1);
  }
  Vendor = (vendor_t *) shmat(shm_id, NULL, 0);
  if ((int) Vendor == -1) {
    printf(
        "%d: failed to attach shared memory to the local memory space! "
        "error: %s\n", 
        __LINE__, 
        strerror(errno));
    exit(1);
  }
  Empty = create_sem("empty", 10);
  Full = create_sem("full", 0);
  Mutex = create_sem("mutex", 1);
  Producer();
  return 0;
}

void Producer() {
  int next;
  int next_empty;
  
  for (next = 0; next < M; next++) {
    sem_wait(Empty);
    sem_wait(Mutex);
    next_empty = Vendor->next_empty; 
    Vendor->slots[next_empty] = next;
    Vendor->next_empty = next_empty == SLOTS - 1 ? 0 : next_empty + 1;
    sem_post(Mutex);
    sem_post(Full);
  }
}

