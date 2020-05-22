#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vendor.h"

#define N 10
#define M 500

sem_t *Empty; 
sem_t *Full; 
sem_t *Mutex; 
vendor_t *Vendor;

void Producer();
void Consumer();
sem_t *create_sem(const char *name, int value);

int main() {
  int i;
  pid_t wpid;
  int status = 0;
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
  for (i = 0; i < N; i++) {
    if (!fork()) {
       Consumer();
    }
  }        
  Producer();
  while ((wpid = wait(&status)) > 0);
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

void Consumer() {
  int num;
  int next_full;

  while (1) {
    sem_wait(Full);
    sem_wait(Mutex);
    next_full = Vendor->next_full;  
    num = Vendor->slots[next_full];
    Vendor->next_full = next_full == SLOTS - 1 ? 0 : next_full + 1;
    printf("%d: %d\n", getpid(), num);
    fflush(stdout);
    sem_post(Mutex);
    sem_post(Empty);
  }
}

sem_t *create_sem(const char *name, int value) {
  sem_t *semaphore;
  // Remove if already exists
  if (sem_unlink(name) == -1) {
    printf(
        "%d: failed to unlink semaphore: %s, error: %s\n", 
        __LINE__,
        name, 
        strerror(errno));
  }
  if ((semaphore = sem_open(name, O_CREAT|O_EXCL, O_RDWR, value)) 
          == SEM_FAILED) {
    printf(
        "%d: failed to create semaphore: %s, error:%s\n", 
        __LINE__,
        name, 
        strerror(errno));
    exit(1);
  }
  return semaphore;
}
