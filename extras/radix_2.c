/*
  Here is a integer based radix sort, will loop n times, where n is the
  number of digits in the largest number
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

void print_array(int arr[], int length) {
  for (int i = 0; i < length; i++)
    printf("%d ", arr[i]);
  printf("\n");
}

int main(void) {
  int input[MAX_LENGTH], output[MAX_LENGTH];
  int count[BASE];
  int i, n, pass, num_digits, max_num = 0;

  // calculate and scan input array
  scanf("%d", &n);
  for (i = 0; i < n; i++) {
    scanf("%d", &input[i]);
    max_num = max(input[i], max_num);
  }
  printf("Original array:\n");
  print_array(input, n);

  printf("\n");
  num_digits = log(max_num) / log(BASE) + 1;
  for (pass = 0; pass < num_digits; pass++) {
    memset(count, 0, BASE * sizeof(int));

    // add counts of each digit per BASE
    for (i = 0; i < n; i++) {
      int d = (input[i] / (int)pow(BASE, pass)) % BASE;
      count[d]++;
    }

    // create cumulative # of digits per idx in arr
    for (i = 1; i < BASE; i++)
      count[i] += count[i - 1];
    // traveling through array in reverse order
    for (i = n - 1; i >= 0; i--) {
      int d = (input[i] / (int)pow(BASE, pass)) % BASE;
      output[count[d] - 1] = input[i];
      count[d] -= 1;
    }
    // update input so that it matches latest sorting
    for (i = 0; i < n; i++)
      input[i] = output[i];
  }
  printf("Sorted array:\n");
  print_array(output, n);
}
