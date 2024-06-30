/*

  Trying to create and test a working version of radix sort
  I'm using this page as a guide: http://codercorner.com/RadixSortRevisited.htm

*/
#include <string.h>

int main(void) {
  unsigned char InputValues[] = {54, 18, 3, 128, 2};
  unsigned char DestinationBuffer[5];
  int i, NbItems = 5;

  int Counters[256];
  memset(Counters, 0, 256 * sizeof(int));
  for (i = 0; i < NbItems; i++) {
    unsigned char c = InputValues[i];
    Counters[c]++;
  }
  // Consists of values greater than 0 where Array Location is item in Input

  int OffsetTable[256];
  OffsetTable[0] = 0;
  for (i = 1; i < 256; i++) {
    OffsetTable[i] = OffsetTable[i - 1] + Counters[i - 1];
  }

  // now, for each input byte, we can get the right offset back thanks to the
  // offset table, put the input byte at the right place in the destination
  // buffer, increase the offset, and repeat the sequence for the next byte

  // for example
  i = 0;
  unsigned char c = InputValues[i];
  DestinationBuffer[OffsetTable[c]++] = c;
  // The destination buffer won't have empty locations, we have enough room for
  // it for the same # of values as the input. InputValues and DestinationBuffer
  // both have size 5

  // With the first character read it's 54,
  // this is inserted in the 3rd location in the Destination Buffer,
  // based on the value in the Offset table
  return 0;
}
