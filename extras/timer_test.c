#include "timer.h"
#include <bits/types/struct_timeval.h>
#include <unistd.h>

int main(void) {
  struct timeval tv1;
  struct timeval tv2;

  get_time(&tv1);
  sleep(3);
  get_time(&tv2);
  print_time(&tv1, &tv2, "3 second sleep");
  return 0;
}
