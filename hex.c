#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define bufsize 4096
void print_hex(char *buffer, ssize_t len) {
  char tmp[bufsize*3];
  for (int i = 0; i < len; i++) {
    tmp[i*3+0] = "0123456789ABCDEF"[buffer[i]/16];
    tmp[i*3+1] = "0123456789ABCDEF"[buffer[i]&15];
    tmp[i*3+2] = ' ';
  }
  write(1, tmp, len*3);
}

int main(int argc, char *argv[]) {
  ssize_t len; 
  char buffer[bufsize];
  int fd;
  if (argc == 1) fd = 0;
  else if ((fd = open(argv[1], O_RDONLY)) == -1) return 1;
  while ((len = read(fd, &buffer, bufsize)) > 0) print_hex(buffer, len);
  return 0;
}
