/*
    Helper functions used to calculate the runtime of function
    calls in this project
*/
#pragma once

#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <sys/time.h>

void get_time(struct timeval *tv) {
  if (tv != NULL) {
    gettimeofday(tv, NULL);
  }
}

void print_time(struct timeval *first, struct timeval *second, char *name) {
  if (first != NULL && second != NULL && name != NULL) {
    double elapsed = (second->tv_sec - first->tv_sec) +
                     (second->tv_usec - first->tv_usec) / 1e6;
    printf("Time to run %s is %f secs\n", name, elapsed);
  }
}
