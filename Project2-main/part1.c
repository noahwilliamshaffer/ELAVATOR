#include <unistd.h>
#include <sys/syscall.h>
int main(void) {
  syscall(SYS_write, 1, "hello, world!\n", 14);
  syscall(SYS_write, 1, "Do Not Disturb!\n", 16);
  syscall(SYS_write, 1, "SMILEY!\n", 8);
  syscall(SYS_write, 1, "WE OUT OUTSIDE\n", 14);
  return 0;
}
