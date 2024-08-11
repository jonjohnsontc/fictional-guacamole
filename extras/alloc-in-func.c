#include <string.h>
void *create_work(char *input) {
  static char copy[256];
  strcpy(copy, input);
  return (void *)copy;
}

int main(void) {}