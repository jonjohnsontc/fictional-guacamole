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

void print_array(int arr[], int length) {
  for (int i = 0; i < length; i++)
    printf("%d ", arr[i]);
  printf("\n");
}

int main(void) {
  int i, n, pass, num_passes, max_num = 0;
  int input[MAX_INPUT];
  int destination[MAX_INPUT];
  char counters[BUF_SIZE]; // count of bytes based on their int identity
  scanf("%d", &n);
  if (n > MAX_INPUT) {
    fprintf(stderr, "input too large to process\n");
    exit(1);
  }
  for (i = 0; i < n; i++) {
    scanf("%d", &input[i]);
    max_num = max(max_num, input[i]);
  }
  printf("Input array:\n");
  print_array(input, n);

  // the number of passes necessary will be based on the item with the largest
  // number of bytes
  num_passes = log((double)max_num) / log((double)BUF_SIZE) + 1;
  // I should be able to store the initial counters values in offsets (or
  // vice-versa) so I can just use one array (outside of the additional return
  // array)

  // main sorting loop; designed to run for however many bytes the largest
  // item is
  for (pass = 0; pass < num_passes; pass++) {
    memset(counters, 0, 256 * sizeof(char));
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
    for (i = 0; i < n; i++) {
      unsigned char radix = (input[i] >> (pass << 3)) & 0x7FF;
      counters[radix]++;
    }
    for (i = 1; i < BUF_SIZE; i++)
      counters[i] += counters[i - 1];
    for (i = n - 1; i >= 0; i--) {
      unsigned char radix = (input[i] >> (pass << 3)) & 0x7FF;
      destination[(int)counters[radix] - 1] = input[i];
      counters[radix] -= 1;
    }
    for (i = 0; i < n; i++)
      input[i] = destination[i];
  }
  printf("After sort\n");
  print_array(input, n);
}
