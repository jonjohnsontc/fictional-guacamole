/*

Taking the notes that I made in radix.c and trying to make a radix sort
that works for an array of integers passed through on stdin on two lines:

- the first line is an integer (N), which is the number of ints on the next line
- the next line contains N integers separated by spaces

The function will return radix sorted integers to stdout

*/
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 256

int main(void) {
  int i, n;
  int counters[BUF_SIZE]; // locations in array give us the count of those idx
                          // in input
  int offsets[BUF_SIZE];  // list of index locations for numbers in input
  scanf("%d", &n);
  // I should be able to store the initial counters values in offsets (or
  // vice-versa) so I can just use one array (outside of the additional return
  // array)
  memset(counters, 0, 256 * sizeof(int));
  for (i = 0; i < n; i++) {
    unsigned char c;
    scanf("%s", &c);
    counters[c]++;
  }
  // may assumption is that this array should be sparse
  offsets[0] = 0;
  for (i = 1; i < BUF_SIZE; i++) {
    offsets[i] = offsets[i - 1] + counters[i - 1];
  }
}
