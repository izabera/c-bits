#include <stdio.h>
#include <string.h>
int main (int argc, char *argv[]) {
  size_t len;
  if (argc < 2 || (len = strlen(argv[1])) == 0) return 1;
  unsigned int i = 0;
  int c;
  while ((c = getchar()) != -1)
    putchar(c^argv[1][i++%len]);
  return 0;
}
