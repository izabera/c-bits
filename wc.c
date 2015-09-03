#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

struct Wc { unsigned int w, m, c, l; };
#define bufsize 4096

struct Wc wc(int fd) { /* counts both utf8 chars and bytes */
  size_t words = 0, chars = 0, bytes = 0, lines = 0;
  bool inword = false;
  unsigned char buffer[bufsize];
  ssize_t len;
  while ((len = read(fd, &buffer, bufsize)) > 0) {
    for (int i = 0; i < len; i++) {
      switch (buffer[i]) {
        case '\n': lines++;
        case ' ': case '\t': case '\r': case '\f': case '\v':
          if (inword) {
            inword = false;
            words++;
          }
          break;
        default: inword = true;
      }
      chars += (buffer[i] < 0x80 || buffer[i] > 0xBF);
    }
    bytes += len;
  }
  return (struct Wc){ words, chars, bytes, lines };
}

enum { flagw = 1, flagc = 2, flagl = 4, flagm = 8 };
void print(struct Wc wc, char *name, int flags) {
  if (flags & flagl) printf("%7d ", wc.l);
  if (flags & flagw) printf("%7d ", wc.w);
  if (flags & flagc) printf("%7d ", wc.c);
  if (flags & flagm) printf("%7d ", wc.m);
  if (name) printf("%s", name);
  putchar('\n');
}

int main(int argc, char *argv[]) {
  int flags = 0, i;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') { /* manual option parsing isn't too bad */
      if (argv[i][1] == '\0') break;
      else {
        if (argv[i][1] == '-' && argv[i][2] == '\0') { i++; break; }
        else
          for (int j = 1; argv[i][j]; j++) {
            switch (argv[i][j]) {
              case 'w': flags |= flagw; break;
              case 'l': flags |= flagl; break;
              case 'c': flags |= flagc; break;
              case 'm': flags |= flagm; break;
              default: return 1; /* helpful messages are bloat */
            }
          }
      }
    } else break;
  }
  if ((flags & flagc) && (flags & flagm)) return 1;
  if (flags == 0) flags = flagl|flagw|flagc;
  if (i == argc) print(wc(0), NULL, flags);
  else {
    bool print_total = (i+1 < argc);
    int fd;
    struct Wc count, tot = { 0 };
    for (; i < argc; i++) {
      if (argv[i][0] == '-' && argv[i][1] == '\0') {
        count = wc(0);
        print(count, argv[i], flags);
        tot.l += count.l; tot.w += count.w; tot.c += count.c; tot.m += count.m;
        continue;
      }
      if ((fd = open(argv[i], O_RDONLY)) == -1) continue;
      count = wc(fd);
      print(count, argv[i], flags);
      close(fd);
      tot.l += count.l; tot.w += count.w; tot.c += count.c; tot.m += count.m;
    }
    if (print_total) print(tot, "total", flags);
  }
  return errno;
}
