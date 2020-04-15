#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N 10 /* Amount of Consumers. */
#define M 500 /* Amount of numbers to write/read. */
/*
 * Maximum amount of digits of an integer (in decimal). It is an upper bound 
 * and it is fine to exceed the exact amount of integer digits. 
 */
#define INT_LEN 10 
#define BASE 10 /* We convert a number to string in decimal. */

const char *stock_file_name = "stock";

static void Producer();
static void Consumer();
static sem_t *create_sem(const char *name, int value);
static int create_new_file(const char *name);
static void seek_first(int fd);
static void seek_last(int fd);
static int read_next_number();
static void itoa(int value, char *str, int base);

sem_t *Empty; 
sem_t *Full; 
sem_t *Mutex; 
int stock;

int main() {
  int i;
  pid_t wpid;
  int status = 0;
  char offset_for_next_read[INT_LEN + 1];

  Empty = create_sem("empty", 10);
  Full = create_sem("full", 0);
  Mutex = create_sem("mutex", 1);
  stock = create_new_file(stock_file_name);
  /* Initialize header. */
  itoa(INT_LEN + 1, offset_for_next_read, BASE);
  if (write(stock, offset_for_next_read, INT_LEN + 1) == -1) {
    printf("main: failed to initialize header! error: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
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
  char num[INT_LEN+1]; 
  int len;
  
  for (next = 0; next < M; next++) {
    sem_wait(Empty);
    sem_wait(Mutex);
    itoa(next, num, BASE);
    len = strlen(num);
    num[len] = '\n';
    seek_last(stock);
    if (write(stock, num, len + 1) == -1) {
      printf(
          "Producer: failed to write to file! error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
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
    /* Read a value and remove that value. */
    num = read_next_number();
    printf("%d: %d\n", getpid(), num);
    fflush(stdout);
    sem_post(Mutex);
    sem_post(Empty);
  }
}

int create_new_file(const char *name) {
  int fd;

  /* Remove the file if it already exists. */
  if (access(name, F_OK) != -1) {
    if (remove(name) == -1) {
      printf(
          "create_new_file: failed to remove file: %s! error:%s\n", 
          name, 
          strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  if ((fd = open(name, O_CREAT|O_RDWR, 0777)) == -1) {
    printf(
        "create_new_file: failed to open file %s! errno:%s\n", 
        name, 
        strerror(errno));
    exit(EXIT_FAILURE);
  }
  return fd;
}

sem_t *create_sem(const char *name, int value) {
  sem_t *semaphore;
  if ((semaphore=sem_open(name, value)) == NULL) {
    printf(
        "create_sem: failed to create semaphore: %s! error:%s\n", 
        name, 
        strerror(errno));
    exit(EXIT_FAILURE);
  }
  return semaphore;
}

void seek_first(int m_file) {
  if (lseek(m_file, 0, SEEK_SET) == -1) {
    printf(
        "seek_first: failed to move to the first char of the file! "
            "errno: %d\n", 
        errno);
    exit(EXIT_FAILURE);
  } 
}

void seek_last(int m_file) {
  if (lseek(m_file, 0, SEEK_END) == -1) {
    printf(
        "seek_last: failed to move to the pos after last char of the file! "
            "error: %s\n", 
        strerror(errno));
    exit(EXIT_FAILURE);
  } 
}

int read_next_number() {
  int i;
  char c[1];
  /* 
   * For reading out the header of the file which indicates the byte position 
   * from which to start reading next. 
   */
  char next[INT_LEN+1]; 
  int next_pos;
  /* For storing the number read out. */
  char num[INT_LEN+1];

  /* Get the next position. */
  seek_first(stock);
  /* INT_LEN + 1 because we read a number and a '\0'. */
  for (i = 0; i < INT_LEN + 1; i++) { 
    if (read(stock, c, 1) == -1) {
      printf(
          "read_next_number: failed to read header! error: %s\n", 
          strerror(errno));
      exit(EXIT_FAILURE);
    }
    next[i] = c[0];
    if (c[0] == '\0') break;
  }
  if (c[0] != '\0') {
    printf(
        "read_next_number: failed to read the next position from header, "
            "header might be  malformated!\n");
    exit(EXIT_FAILURE);
  }
  next_pos = atoi(next);
  if (lseek(stock, next_pos, SEEK_SET) == -1) {
    printf(
        "read_next_number: failed to set the position to byte: %d,"
            "error: %s\n", 
        next_pos, 
        strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* Read the next number. */
  for (i = 0; i < INT_LEN + 1; i++) {
    if (read(stock, c, 1) == -1) {
      printf("read_next_number: failed to read! error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    num[i] = c[0];
    if (c[0] == '\n') break;
  }
  num[i] = '\0';
  /* Update header for the next position. */
  next_pos = next_pos + i + 1;
  itoa(next_pos, next, BASE);
  seek_first(stock);
  if (write(stock, next, INT_LEN + 1) == -1) {
    printf(
        "read_next_number: failed to update header! error: %s\n", 
        strerror(errno));
    exit(EXIT_FAILURE);
  }

  return atoi(num);
}

void itoa(int value, char *str, int base) {
  int i, j;
  int tmp;
  int quotient = value;
  int remainder;

  if (value == 0) {
    str[0] = '0';
    str[1] = '\0';
    return;
  }
  if (base > 10) {
    printf("itoa: base can only be <= 10 for now!");
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < INT_LEN && quotient != 0; i++) {
    remainder = quotient % base;
    quotient = quotient / base;
    str[i] = remainder + '0'; 
  }
  if (quotient != 0) {
    printf("itoa: shouldn't reach here!");
    exit(EXIT_FAILURE);
  }
  str[i] = '\0';
  for (j = 0; j < i/2; j++) {
    tmp = str[j];
    str[j] = str[i-1-j];
    str[i-1-j] = tmp;
  }
}
