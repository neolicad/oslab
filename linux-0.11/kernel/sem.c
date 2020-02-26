/*
 *  linux/kernel/sem.c
 *
 *  (C) 2020  Neolicad
 */ 

#include <asm/segment.h>
#include <linux/kernel.h>
#include <linux/sem.h>
#include <errno.h>

#define NAME_LIMIT 30

static int fetch_name(char *dest, const char *src);

struct sem_t *sys_sem_open(const char *name, unsigned int value) 
{ 
  char buf[NAME_LIMIT];
  if (fetch_name(buf, name) < 0) {
    return -1;
  }
  printk("sys_semopen! name: %s, value: %d\n", buf, value); 
  return NULL;
} 

int sys_sem_wait(struct sem_t *sem) 
{
  printk("sys_semwait!\n");
  return 0;
} 

int sys_sem_post(struct sem_t *sem)
{
  printk("sys_sempost!\n");
  return 0;
} 

int sys_sem_unlink(const char *name)
{
  char buf[NAME_LIMIT];
  if (fetch_name(buf, name) < 0) {
    return -1;
  }
  printk("sys_semunlink! name: %s\n", buf);
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
    errno = ENAMETOOLONG;
    return -1;
  }
  return 0;
}
