/*
  I would like to mmap an input file and read a couple different parts
  of it afterwards.
*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define INPUT_FILE "./measurements_1b.txt"
#define err_abort(code, text)                                                  \
  do {                                                                         \
    fprintf(stderr, "%s at \"%s\":%d:%s\n", text, __FILE__, __LINE__,          \
            strerror(code));                                                   \
    abort();                                                                   \
  } while (0)

int main(void) {
  int fd;
  char *addr;
  struct stat sb;
  off_t size;

  fd = open(INPUT_FILE, O_RDONLY);
  if (fd == -1)
    err_abort(fd, "file open");

  size = lseek(fd, 0, SEEK_END);
  if (size == -1)
    err_abort(size, "lseek");

  if (fstat(fd, &sb) == -1)
    err_abort(-1, "fstat");

  printf("file size is %ld bytes\n", sb.st_size);
  printf("file size is %ld bytes from lseek\n", size);
  addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED)
    err_abort(*addr, "mmap");

  close(
      fd); // closing the file early to make sure we can still access the chars

  printf("file has been mmapped\n");
  printf("preview: %.11s\n", addr);
  munmap(addr, sb.st_size);
  return 0;
}