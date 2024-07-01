/*
  I am feeling extremely stupid tyring to get the byte based radix sort approach
  working I was thinking I could work on a method that worked on base 10 numbers
*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#define MAX_LENGTH 65535
#define BASE 10
int max(int a, int b) {
  if (a > b)
    return a;
  return b;
}

int main(void) {
  int input[MAX_LENGTH], output[MAX_LENGTH];
  int count[BASE], offset[BASE];
  int i, n, pass, num_digits, max_num = 0;

  // calculate and scan input array
  scanf("%d", &n);
  for (i = 0; i < n; i++) {
    scanf("%d", &input[i]);
    max_num = max(input[i], max_num);
  }
  num_digits = log(max_num) / log(BASE) + 1;
  memset(count, 0, 10 * sizeof(int));
  for (pass = 0; pass < num_digits; pass++) {

    // add counts of each digit per BASE
    for (i = 0; i < n; i++) {
      int d = (input[i] / (int)pow(BASE, pass)) % BASE;
      count[i] = count[d] + 1;
    }

    offset[0] = 0;
    // create cumulative # of digits per idx in arr
    for (i = 1; i < BASE; i++)
      offset[i] = offset[i - 1] + count[i - 1];

    // traveling through array in reverse order
    for (i = n - 1; i >= -1; i--) {
      int d = (input[i] / (int)pow(BASE, pass)) % BASE;
      offset[d] -= 1;
      output[offset[i]] = input[i];
    }
  }
  printf("Sorted array:\n");
  for (i = 0; i < n; i++)
    printf("%d ", output[i]);
}
