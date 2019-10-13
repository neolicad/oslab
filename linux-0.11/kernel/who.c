#include<asm/segment.h>
#include<errno.h>

const int MAX_NAME_LENGTH=23;
char NAME[MAX_NAME_LENGTH + 1];

/*
 * Copy to kernel the string in name if the string length is not greater
 * than 23, return the length of name if not greater than 23, or -1 (with 
 * errno EINVAL) if length is greater than 23.
 */
int sys_iam(char* name) {
  int pos;
  char c;
  for (
      pos = 0, c = get_fs_byte(name); 
      pos < MAX_NAME_LENGTH && c != '\0'; 
      pos++, c = get_fs_byte(name)) {
    NAME[pos] = c;
  }
  name[pos] = '\0';
  if (pos == MAX_NAME_LENGTH && c != '\0') {
    errno = EINVAL;
    return -1;
  }
  return pos;
}

/*
 * Copy the string written by sys_iam() from kernel to the user space 
 * specified in name, making sure the length of the string <= size. Return 
 * length of string if length <= size, or else return -1 with errno EINVAL. 
 */
int sys_whoami(char* name, unsigned int size) {
  int length = strlen(NAME);
  if (length > size) {
    errno = EINVAL;
    return -1;
  }
  int i;
  for (i = 0; i < length; i++) {
    put_fs_byte(NAME[i], name + i);
  }
  return length;
}
