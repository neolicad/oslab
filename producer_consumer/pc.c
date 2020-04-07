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

#define N 10
#define M 500
#define INT_LEN 10

sem_t *Empty; 
sem_t *Full; 
sem_t *Mutex; 

void Producer();
void Consumer();
sem_t *create_sem(const char *name, int value);
int create_new_file(const char *name);
int read_next();
void seek_first(int);
void seek_last(int);
void itoa(int value, char *str, int base);

const char *stock_file_name = "stock";
int stock;

int main() {
  int i;
  pid_t wpid;
  int status = 0;
  char header[INT_LEN+1];

  Empty = create_sem("empty", 10);
  Full = create_sem("full", 0);
  Mutex = create_sem("mutex", 1);
  stock = create_new_file(stock_file_name);
  /* Initialize header. */
  itoa(INT_LEN+1, header, 10);
  if (write(stock, header, INT_LEN+1) == -1) {
    printf("main: failed to initialize header! error: %s\n", strerror(errno));
  }
  if (!fork()) {
    for (i = 0; i < N - 1; i++) {
      if (!fork()) {
         Consumer();
      }
    }        
    Consumer();
  }
  Producer();
  while ((wpid = wait(&status)) > 0);
  return 0;
}

void Producer() {
  int next;
  char num[INT_LEN+1]; 
  int len;
  
  for (next = 0; next < M; next++) {
    sem_wait(Empty);
    sem_wait(Mutex);
    itoa(next, num, 10);
    len = strlen(num);
    num[len] = '\n';
    seek_last(stock);
    if (write(stock, num, len+1) == -1) {
      printf(
          "Producer: failed to write to file! error: %s\n", strerror(errno));
      exit(-1);
    }
    sem_post(Mutex);
    sem_post(Full);
  }
}

void Consumer() {
  int num;

  while (1) {
    sem_wait(Full);
    sem_wait(Mutex);
    // Read a value and remove that value.
    num = read_next();
    printf("%d: %d\n", getpid(), num);
    fflush(stdout);
    sem_post(Mutex);
    sem_post(Empty);
  }
}

int create_new_file(const char *name) {
  int fd;
  /* Remove the file if exists. */
  if (access(name, F_OK) != -1) {
    if (remove(name) == -1) {
      printf("fail to remove file: %s! error:%s\n", name, strerror(errno));
      exit(errno);
    }
  }
  umask(0);
  if ((fd = open(name, O_CREAT|O_RDWR, 0777)) == -1) {
    printf(
        "create_file: fail to open file %s! errno:%s\n", 
        name, 
        strerror(errno));
    exit(errno);
  }
  return fd;
}

sem_t *create_sem(const char *name, int value) {
  sem_t *semaphore;
  if (sem_unlink(name) == -1) {
    printf("fail to unlink semaphore: %s, error: %s\n", name, strerror(errno));
    exit(-1);
  }
  if ((semaphore = sem_open(name, O_CREAT|O_EXCL, O_RDWR, value)) 
          == SEM_FAILED) {
    printf(
        "Failed to create semaphore: %s, error:%s\n", name, strerror(errno));
    exit(-1);
  }
  return semaphore;
}

void seek_first(int m_file) {
  if (lseek(m_file, 0, SEEK_SET) == -1) {
    printf("Failed to move to the first char of the file! errno: %d\n", errno);
    exit(-1);
  } 
}

void seek_last(int m_file) {
  if (lseek(m_file, 0, SEEK_END) == -1) {
    printf(
        "Failed to move to the pos after last char of the file! error: %s\n", 
        strerror(errno));
    exit(-1);
  } 
}

int read_next() {
  int i;
  char c[1];
  char next[INT_LEN+1];
  int next_pos;
  char first[INT_LEN+1];

  /* Get the next position. */
  seek_first(stock);
  for (i = 0; i < INT_LEN + 1; i++) {
    if (read(stock, c, 1) == -1) {
      printf("Failed to read header! error: %s\n", strerror(errno));
      exit(-1);
    }
    next[i] = c[0];
    if (c[0] == '\0') break;
  }
  if (c[0] != '\0') {
    printf(
        "Failed to read the next position from header, header might be \
            malformated!\n");
    exit(-1);
  }
  next_pos = atoi(next);
  lseek(stock, next_pos, SEEK_SET);
  /* Read the next number. */
  for (i = 0; i < INT_LEN + 1; i++) {
    if (read(stock, c, 1) == -1) {
      printf("Failed to read! errno: %d\n", errno);
      exit(errno);
    }
    first[i] = c[0];
    if (c[0] == '\n') break;
  }
  first[i] = '\0';
  /* Update header for the next position. */
  next_pos = next_pos + i + 1;
  itoa(next_pos, next, 10);
  lseek(stock, 0, SEEK_SET);
  if (write(stock, next, INT_LEN+1) == -1) {
    printf("read_next: failed to update header! error: %s\n", strerror(errno));
  }

  return atoi(first);
}

void itoa(int value, char *str, int base) {
  int i, j;
  int tmp;
  int quotient = value;
  int remainder;
  if (base > 10) {
    printf("itoa: base can only be <= 10 for now!");
    exit(EINVAL);
  }
  if (value < 0) {
    printf("itoa: value can only be >= 0 for now!");
    exit(EINVAL);
  }
  if (value == 0) {
    str[0] = '0';
    str[1] = '\0';
    return;
  }
  for (i = 0; i < INT_LEN && quotient != 0; i++) {
    remainder = quotient % base;
    quotient = quotient / base;
    str[i] = remainder + '0'; 
  }
  if (quotient != 0) {
    printf("itoa: number is too big that the provider buffer cannot hold!");
    exit(EINVAL);
  }
  str[i] = '\0';
  for (j = 0; j < i/2; j++) {
    tmp = str[j];
    str[j] = str[i-1-j];
    str[i-1-j] = tmp;
  }
}
