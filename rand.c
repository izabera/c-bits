#include <unistd.h>
#include <stdint.h>

uint64_t atoi(const char *s) {
  uint64_t n = 0;
  while (*s >= '0' && *s <= '9') n = 10*n + (*s++ - '0');
  return n;
}

uint64_t state; 
/* http://www.pcg-random.org */
uint32_t pcg32_random() {
  state = state * 6364136223846793005ULL + 1;
  uint32_t xorshifted = ((state >> 18) ^ state) >> 27, rot = state >> 59u;
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

#define bufsize 4096
int main(int argc, char *argv[]) {
  uint32_t buf[bufsize], count = argc > 1 ? atoi(argv[1]) : 100, i;
  state = (argc > 2) ? atoi(argv[2]) : 42;
  while (count--) {
    for (i = 0; i < bufsize / sizeof(uint32_t); i++) buf[i] = pcg32_random();
    write(1, buf, bufsize);
  }
  return 0;
}
