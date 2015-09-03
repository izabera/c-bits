#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

void print_file(int fd) {
#define bufsize 4096
  char buffer[bufsize];
  ssize_t len;
  while ((len = read(fd, &buffer, bufsize)) > 0) write(1, buffer, len);
}

int main(int argc, char *argv[]) {
  int i;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == '\0') break;
      else {
        if (argv[i][1] == '-' && argv[i][2] == '\0') { i++; break; }
        else
          for (int j = 1; argv[i][j]; j++) {
            switch (argv[i][j]) {
              case 'u': break;
              default: return 1;
            }
          }
      }
    } else break;
  }
  if (i == argc) print_file(0);
  else {
    int fd;
    for (; i < argc; i++) {
      if (argv[i][0] == '-' && argv[i][1] == '\0') {
        print_file(0);
        continue;
      }
      if ((fd = open(argv[i], O_RDONLY)) == -1) continue;
      print_file(fd);
      close(fd);
    }
  }
  return errno;
}
