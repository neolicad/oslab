#include <asm/segment.h>
#include <errno.h>
#include <string.h>

static const int MAX_NAME_LENGTH = 23;
static char NAME[24];

/*
 * Copy to kernel the string in name if the string length is not greater
 * than 23, return the length of name if not greater than 23, or -EINVAL if 
 * length is greater than 23.
 */
int sys_iam(const char* name) {
  int pos;
  char c;
  for (
      pos = 0; 
      pos < MAX_NAME_LENGTH 
          && (c = get_fs_byte(name + pos)) != '\0'; 
      pos++) {
    NAME[pos] = c;
  }
  NAME[pos] = '\0';
  if (pos == MAX_NAME_LENGTH 
      && get_fs_byte(name + MAX_NAME_LENGTH) != '\0') {
    return -EINVAL;
  }
  return pos;
}

/*
 * Copy the string written by sys_iam() from kernel to the user space memory 
 * specified in name, making sure the length of the string <= size. Return 
 * length of string if length <= size, or else return -EINVAL. 
 */
int sys_whoami(char* name, unsigned int size) {
  int length = strlen(NAME);
  if (length > size) {
    return -EINVAL;
  }
  int i;
  // NAME[length] == '\0'
  for (i = 0; i <= length; i++) {
    put_fs_byte(NAME[i], name + i);
  }
  return length;
}
