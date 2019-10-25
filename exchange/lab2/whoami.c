#include <stdio.h>

int main(int argc, char** argv) {
  char out[24];
  whoami(out, 23);
  printf("%s\n", out);
}
