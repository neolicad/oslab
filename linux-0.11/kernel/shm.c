#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <sys/types.h>
#include <errno.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif
#define SHARED_MEMORY_SIZE 10

/*
 * Caveat: If multiple processes share the same physical memory - which is what 
 * this shared memory module is used for - and one of the processes exits, 
 * the physical page will be marked unused and it will be reused unexpected. In 
 * other words, the behavior is incorrect when one of the shared memory exits, 
 * and this module only works when all the sharing processes are alive. 
 * I haven't fully gone through the whole page lifecycle, but to make this 
 * module more general, we may need to:
 * 1. Have in mem_map a notion to show that it is a shared memory.  
 * 2. When a process exits, do not throw a kernel panic (in exit.c) if mem_map 
 * is not 1 but the page is a shared one.
 * 3. Do not use put_page here, because it is intended to be used for attaching 
 * a newly created (not yet attached to the linear space) page. Have a 
 * dedicated logic here for attaching the shared memory to user space.
 */

typedef struct shared_memory {
  unsigned int key;
  size_t size;
  unsigned long page; /* physical address of the corresponding page */
} shared_memory_t;

shared_memory_t *shms[SHARED_MEMORY_SIZE];

/*
 * Creates (if not exist) or gets (if already exists) the shared memory with 
 * the specified key and size. 
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
  /* The shared memory keyed on `key` already exists, return the index; */
  if (i != SHARED_MEMORY_SIZE) {
    return i;
  }
  /* Otherwise, create one. */
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

/*
 * Attaches to the local user space the shared memory represented by the i-th 
 * item in shms. Returns the logical address of the attached memory if 
 * succeeds, or errno if failes. 
 */
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
