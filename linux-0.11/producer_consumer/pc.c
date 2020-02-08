#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define N 10
#define M 500
#define BUFFER_SIZE 4

sem_t *Empty; 
sem_t *Full; 
sem_t *Mutex; 
int stock;
pid_t wpid;
int status = 0;

void Producer();
void Consumer();
sem_t *create_sem(const char *name, int value);
int create_new_file(const char *name);
int read_and_delete_first_line(int, const char*);
void seek_first(int);
void seek_last(int);

const char *stock_file_name = "stock";

int main() {
  int i;
  Empty = create_sem("empty", 10);
  Full = create_sem("full", 0);
  Mutex = create_sem("mutex", 1);
  stock = create_new_file(stock_file_name);
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

void remove_if_exists(const char *name) {
  if (access(name, F_OK) != -1) {
    if (remove(name) == -1) {
      printf("fail to remove file: %s! error:%s\n", name, strerror(errno));
      exit(errno);
    }
  }
}

int create_new_file(const char *name) {
  int fd;
  remove_if_exists(name);
  if ((fd = open(name, O_CREAT|O_RDWR)) == -1) {
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
    exit(errno);
  }
  if ((semaphore = sem_open(name, O_CREAT|O_EXCL, O_RDWR, value)) 
          == SEM_FAILED) {
    printf(
        "Failed to create semaphore: %s, error:%s\n", name, strerror(errno));
    exit(errno);
  }
  return semaphore;
}

void Producer() {
  int next;
  char buffer[BUFFER_SIZE]; 
  int len;
  for (next = 0; next < M; next++) {
    sem_wait(Empty);
    sem_wait(Mutex);
    // Write to file
    snprintf(buffer, BUFFER_SIZE, "%d", next);
    len = strlen(buffer);
    buffer[len] = '\n';
    seek_last(stock);
    if(write(stock, buffer, len+1) == -1) {
      printf(
          "Producer: failed to write to file! errnor: %s\n", strerror(errno));
      exit(errno);
    }
    sem_post(Mutex);
    sem_post(Full);
  }
}

void Consumer() {
  char buffer[BUFFER_SIZE];
  while (1) {
    sem_wait(Full);
    sem_wait(Mutex);
    // Read a value and remove that value
    int num = read_and_delete_first_line(stock, stock_file_name);
    printf("%d: %d\n", getpid(), num);
    fflush(stdout);
    sem_post(Mutex);
    sem_post(Empty);
  }
}

void seek_first(int m_file) {
  if (lseek(m_file, 0, SEEK_SET) == -1) {
    printf("fail to move to the first char of the file! errno: %d\n", errno);
    exit(errno);
  } 
}

void seek_last(int m_file) {
  if (lseek(m_file, 0, SEEK_END) == -1) {
    printf(
        "fail to move to the pos after last char of the file! errno: %d\n", 
        errno);
    exit(errno);
  } 
}

int read_and_delete_first_line(int m_file, const char *name) {
  int tmp;
  int i;
  char c[1];
  char first[BUFFER_SIZE];
  int first_num;
  tmp = create_new_file("tmp");
  // Find first line.
  seek_first(m_file);
  // We should stop at the maximum digit number +1, e.g. if M=500, we should 
  // stop at 4, if we don't find '\n' after reading 4 chars, something went 
  // wrong. Just set a bigger upper bound here (I slack off ¯\_(ツ)_/¯). 
  for (i = 0; i < BUFFER_SIZE ;i++) {
    if (read(m_file, c, 1) == -1) {
      printf("fail to read! errno:%d\n", errno);
      exit(errno);
    }
    first[i] = c[0];
    if (c[0] == '\n') break;
  }
  first[i] = '\0';
  first_num = atoi(first);
  // Copy cotent of m_file to tmp, with first line deleted.
  while (1) {
    int res = read(m_file, c, 1);
    if (res == -1) {
      printf("fail to read! errno:%d\n", errno);
      exit(errno);
    }
    if (res == 0) break;
    if (write(tmp, c, 1) == -1) {
      printf("fail to write! errno:%d\n", errno);
      exit(errno);
    }
  }
  // Copy content of tmp back to m_file.
  seek_first(tmp);
  seek_first(m_file);
  for (i = 0; ; i++) {
    int res = read(tmp, c, 1);
    if (res == -1) {
      printf("fail to read! errno:%d\n", errno);
      exit(errno);
    }
    if (res == 0) break;
    if (write(m_file, c, 1) == -1) {
      printf("fail to write! errno:%d\n", errno);
      exit(errno);
    }
  }
  if (ftruncate(m_file, i) == -1) {
    printf("fail to truncate file! error: %s\n", strerror(errno));
    exit(errno);
  }
  if (remove("tmp") == -1) {
    printf("fail to remove file: %s! errno:%d\n", name, errno);
    exit(errno);
  }
  return first_num;
}

