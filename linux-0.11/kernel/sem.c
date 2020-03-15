/*
 *  linux/kernel/sem.c
 *
 *  (C) 2020  Neolicad
 */ 

#include <asm/segment.h>
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sem.h>
#include <errno.h>

#define NAME_LIMIT 30
#define NUM_SEMS 10

static int fetch_name(char *dest, const char *src);

struct sem_t *sems[NUM_SEMS];

int sys_sem_create(const char *name, unsigned int value) 
{ 
  char buf[NAME_LIMIT];
  int i;
  int sem_pos = -1;
  char *sem_name;
  int err;
  if ((err=fetch_name(buf, name)) < 0) {
    return err;
  }
  cli();
  // Check if the corresponding semaphore already exists.
  for (i = 0; i < NUM_SEMS; i++) {
    if (sems[i] == NULL) {
      continue;
    }
    if (strcmp(sems[i]->name, buf) == 0) {
      sem_pos = i;
      break;
    }
  }
  // The corresponding semaphore does not exist, create one.
  if (sem_pos == -1) {
    struct sem_t *sem;
    for (i = 0; i < NUM_SEMS; i++) {
      if (sems[i] == NULL) {
        sem_pos = i;
        break;
      }
    } 
    if (sem_pos == -1) {
      sti();
      return -ENOENT;
    }
    sem = (struct sem_t *) malloc(sizeof(struct sem_t));
    sem_name = (char *) malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(sem_name, buf);
    sem->name = sem_name;
    sem->queue = NULL;
    sem->value = value;
    sem->token = 0;
    sems[sem_pos] = sem;
  } 
  sti();
  return sem_pos;
} 

int sys_sem_P(int i) 
{
  struct sem_t *sem;
  if (i < 0 || i >= NUM_SEMS) {
    return -ERANGE;
  }
  cli();
  sem = sems[i];
  if (sem == NULL) {
    sti();
    return -EINVAL;
  }
  sem->value--;
  if (sem->value < 0) {
    while (sem->token == 0) {
      sleep_on(&sem->queue);
    }
  }
  sem->token = 0;
  sti();
  return 0;
} 

int sys_sem_V(int i)
{  
  struct sem_t *sem;
  if (i < 0 || i >= NUM_SEMS) {
    return -ERANGE;
  }
  cli();
  sem = sems[i];
  if (sem == NULL) {
    sti();
    return -EINVAL;
  }
  sem->value++;
  if (sem->value >= 0) {
    sem->token = 1;
    wake_up(&sem->queue);
  }
  sti();
  return 0;
} 

int sys_sem_unlink(const char *name)
{
  char buf[NAME_LIMIT];
  int err;
  struct sem_t *sem = NULL;
  int sem_pos = -1;
  int i;
  if ((err=fetch_name(buf, name)) < 0) {
    return err;
  }
  cli();
  // find the semaphore with the name $name
  for (i = 0; i < NUM_SEMS; i++) {
    if (strcmp(sems[i]->name, buf) == 0) {
      sem_pos = i;
      sem = sems[i];
      break;
    }
  }
  if (sem_pos == -1) {
    sti();
    return -ENOENT;
  }
  free_s(sem->name, (strlen(sem->name)+1) * sizeof(char));
  free_s(sem, sizeof(struct sem_t));
  sems[sem_pos] = NULL;
  sti();
  return 0;
} 

int fetch_name(char *dest, const char *src) {
  int i;
  char r;
  for (i = 0; i < NAME_LIMIT; i++) {
    r = get_fs_byte(src + i);
    dest[i] = r;
    if (r == '\0') break;
  }
  if (r != '\0') {
    return -ENAMETOOLONG;
  }
  return 0;
}
