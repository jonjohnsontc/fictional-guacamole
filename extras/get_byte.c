#include <stdio.h>
// Function to get a specific byte of an integer. I pulled this from ChatGPT
int get_byte(int num, int byte_num) { return (num >> (8 * byte_num)) & 0xFF; }

int main(void) {
  int test = 511232;
  int first = get_byte(test, 0);
  int second = get_byte(test, 1);
  printf("First is %d\n", first);
  printf("Second is %d\n", second);
}
