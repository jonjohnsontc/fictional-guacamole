/*
  Copied directly from ChatGPT, original text:

  How would I perform radix sort in C on an array of integers,
  using byte based counting sort?
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get a specific byte of an integer
int get_byte(int num, int byte_num) { return (num >> (8 * byte_num)) & 0xFF; }

// Counting sort based on a specific byte
void counting_sort_byte(int arr[], int n, int byte_num) {
  int output[n];
  int count[256] = {0};

  // Store the count of occurrences of each byte value
  for (int i = 0; i < n; i++) {
    int byte_value = get_byte(arr[i], byte_num);
    count[byte_value]++;
  }

  // Change count[i] so it contains the actual position of this byte in output[]
  for (int i = 1; i < 256; i++) {
    count[i] += count[i - 1];
  }

  // Build the output array
  for (int i = n - 1; i >= 0; i--) {
    int byte_value = get_byte(arr[i], byte_num);
    output[count[byte_value] - 1] = arr[i];
    count[byte_value]--;
  }

  // Copy the output array to arr[], so that arr[] contains sorted numbers
  for (int i = 0; i < n; i++) {
    arr[i] = output[i];
  }
}

// Radix sort
void radix_sort(int arr[], int n) {
  for (int byte_num = 0; byte_num < 4; byte_num++) {
    counting_sort_byte(arr, n, byte_num);
  }
}

// Helper function to print an array
void print_array(int arr[], int n) {
  for (int i = 0; i < n; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
}

int main() {
  int arr[] = {170, 45, 75, 90, 802, 24, 2, 66};
  int n = sizeof(arr) / sizeof(arr[0]);

  printf("Original array: \n");
  print_array(arr, n);

  radix_sort(arr, n);

  printf("Sorted array: \n");
  print_array(arr, n);

  return 0;
}
