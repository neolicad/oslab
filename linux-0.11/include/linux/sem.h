#ifndef NULL
#define NULL ((void *) 0)
#endif

struct sem_t {
  int value;
  char token;
  struct task_struct *queue;
  const char *name;
};
