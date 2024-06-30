/*

Taking the notes that I made in radix.c and trying to make a radix sort
that works for an array of integers passed through on stdin on two lines:

- the first line is an integer (N), which is the number of ints on the next line
- the next line contains N integers separated by spaces

The function will return radix sorted integers to stdout

Also taking notes from https://brilliant.org/wiki/radix-sort/

*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 256
#define MAX_INPUT 65553

int max(int a, int b) {
  if (a > b)
    return a;
  return b;
}

int main(void) {
  int i, n, pass, num_passes, max_num = 0;
  int input[MAX_INPUT];
  int destination[MAX_INPUT];
  char counters[BUF_SIZE]; // locations in array give us the count of those idx
                           // in input
  char offsets[BUF_SIZE];  // list of index locations for numbers in input
  scanf("%d", &n);
  if (n > MAX_INPUT) {
    fprintf(stderr, "input too large to process\n");
    exit(1);
  }
  for (i = 0; i < n; i++) {
    scanf("%d", &input[i]);
    max_num = max(max_num, input[i]);
  }

  // the number of passes necessary will be based on the item with the largest
  // number of bytes
  num_passes = log((double)max_num) / log((double)BUF_SIZE) + 1;
  printf("Number of passes necessary %d\n", num_passes);
  // I should be able to store the initial counters values in offsets (or
  // vice-versa) so I can just use one array (outside of the additional return
  // array)
  memset(counters, 0, 256 * sizeof(char));

  // main sorting loop; designed to run for however many bytes the largest
  // item is
  for (pass = 0; pass < num_passes; pass++) {
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
    }
    for (i = 1; i < BUF_SIZE; i++)
      offsets[i] = offsets[i - 1] + counters[i - 1];
    for (i = 0; i < n; i++) {
      unsigned char radix = (input[i] >> (pass << 3)) & 0x7FF;
      destination[(int)offsets[radix]++] = input[i];
    }
    printf("After pass %d, destination looks like:\n", pass);
    for (i = 0; i < n; i++)
      if (i + 1 < n)
        printf("%d, ", destination[i]);
      else
        printf("%d\n", destination[i]);
  }
}
