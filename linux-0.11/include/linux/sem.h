#ifndef NULL
#define NULL ((void *) 0)
#endif

typedef struct Sem {
  int value;
  int wakeup;
  struct task_struct *queue;
  const char *name;
} sem_t;
