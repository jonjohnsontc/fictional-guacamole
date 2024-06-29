/*

Taking the notes that I made in radix.c and trying to make a radix sort
that works for an array of integers passed through on stdin on two lines:

- the first line is an integer (N), which is the number of ints on the next line
- the next line contains N integers separated by spaces

The function will return radix sorted integers to stdout

*/
#include <stdio.h>
#define MAX_NUMS 655536

int main(void) {
  int i, n;
  int nums[MAX_NUMS];
  int offsets[MAX_NUMS];

  scanf("%d", &n);
  for (i = 0; i < n; i++) {
    scanf("%d", &nums[i]);
  }
}