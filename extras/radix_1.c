/*

Taking the notes that I made in radix.c and trying to make a radix sort
that works for an array of integers passed through on stdin on two lines:

- the first line is an integer (N), which is the number of ints on the next line
- the next line contains N integers separated by spaces

The function will return radix sorted integers to stdout

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 256
#define MAX_INPUT 655535
#define NUM_DIGITS 2 // TODO: make this user adjustable

int main(void) {
  int i, n, pass;
  int input[MAX_INPUT];
  int destination[MAX_INPUT];
  int counters[BUF_SIZE]; // locations in array give us the count of those idx
                          // in input
  int offsets[BUF_SIZE];  // list of index locations for numbers in input
  scanf("%d", &n);
  if (n > MAX_INPUT) {
    fprintf(stderr, "input too large to process\n");
    exit(1);
  }
  for (i = 0; i < n; i++) {
    scanf("%d", &input[i]);
  }
  // I should be able to store the initial counters values in offsets (or
  // vice-versa) so I can just use one array (outside of the additional return
  // array)
  memset(counters, 0, 256 * sizeof(int));

  // main sorting loop; designed to run for however many bytes the largest
  // item is
  for (pass = 0; pass < NUM_DIGITS; pass++) {
    // grab pass numbered byte in input
    /*
      At each pass, what are my assumptions?
        - first pass:
          -counters are at all zeroes
          -offsets are at garbage values
        - second pass:
          -counters are at valid values (1s) for all of the numbers in the
            input array, because they're all one byte max
        - third pass:
          - not sure, maybe counters + offset are all 0's?
    */
    offsets[0] = 0;
    for (i = 0; i < n; i++) {
      unsigned char radix = (input[i] >> (pass << 3)) & 0x7FF;
      counters[radix]++;
      if (i >= 1)
        offsets[i] = offsets[i - 1] + counters[i - 1];
    }
    for (i = 0; i < n; i++) {
      unsigned char radix = (input[i] >> (pass << 3)) & 0x7FF;
      destination[offsets[radix]++] = radix;
    }
  }
}
