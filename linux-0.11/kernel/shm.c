#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <sys/types.h>
#include <errno.h>

#define NULL ((void *) 0)
#define SHARED_MEMORY_SIZE 10

typedef struct shared_memory {
  unsigned int key;
  size_t size;
  unsigned long page;
} shared_memory_t;

shared_memory_t *shms[SHARED_MEMORY_SIZE];

/**
 * Creates (if not exist) or gets (if already exists) the shared memory with 
 * speicified key and size. 
 * Return the index of the shared memory in shms if succeeds, or the errno if 
 * fails.
 */
int sys_shmget(unsigned int key, size_t size, int shmflg) {
  int i;
  unsigned long page;
  if (size < 0) {
    return -EINVAL;
  }
  if (size > PAGE_SIZE) {
    return -E2BIG;
  }

  for (i = 0; i < SHARED_MEMORY_SIZE; i++) {
    if (shms[i] != NULL && shms[i]->key == key) {
      break;
    }
  }
  // Shared memory with key already exists, return the index;
  if (i != SHARED_MEMORY_SIZE) {
    return i;
  }
  // Otherwise, create one.
  for (i = 0; i < SHARED_MEMORY_SIZE; i++) {
    if (shms[i] == NULL) {
      break; 
    }
  }
  if (i == SHARED_MEMORY_SIZE) {
    return -EINVAL;
  }
  if (!(page=get_free_page())) {
    return -ENOMEM;
  }
  shms[i] = (shared_memory_t *) malloc(sizeof(shared_memory_t));
  shms[i]->page = page;
  shms[i]->key = key;
  shms[i]->size = size;
  return i;
}

void *sys_shmat(int i, const void *shmaddr, int shmflg) {
  unsigned long address;
  if (i < 0) {
    return -EINVAL;
  }
  if (i >= SHARED_MEMORY_SIZE) {
    return -E2BIG;
  }
  if (shms[i] == NULL) {
    return -ENOENT;
  }
  /* Make sure the address is aligned with PAGE_SIZE. */
  address = (current->brk + PAGE_SIZE - 1) & 0xfffff000;
  if (address + PAGE_SIZE >= current->start_stack - 16384) {
    return -ERANGE;
  }
  current->brk = address + PAGE_SIZE;
  /* 
   * start_code is the start of the current process's memory (linear address). 
   */
  if (!put_page(shms[i]->page, current->start_code + address)) {
    return -ENOMEM;
  }
  return (void *) address;
} 
