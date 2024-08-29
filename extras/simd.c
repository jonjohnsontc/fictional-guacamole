#include "simd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
        Trying to get an understanding of which SIMD / instrinsic
        functions are available for my system, and  how to best use
        them.
*/
int main(void) {
  float arr_1[] = {2, 3, 7, 72};
  float arr_2[] = {8, 7, 2, 12};

  __m128 loaded1 = _mm_loadu_ps((const float *)&arr_1);
  __m128 loaded2 = _mm_loadu_ps((const float *)&arr_2);
  __m128 result = _mm_add_ps(loaded1, loaded2);

  float arr_3[4];
  _mm_store_ps((float *)&arr_3, result);

  assert(arr_3[0] == 10.0);
  assert(arr_3[1] == 10.0);
  assert(arr_3[2] == 9.0);
  assert(arr_3[3] == 84.0);

  float *arr_4 = malloc(sizeof(float) * 4);
  if (arr_4 == NULL) {
    fprintf(stderr, "malloc err\n");
    exit(1);
  }

  x4_add(&arr_4, arr_1, arr_2);
  assert(arr_4[0] == 10.0);
  assert(arr_4[1] == 10.0);
  assert(arr_4[2] == 9.0);
  assert(arr_4[3] == 84.0);

  float *arr_5 = malloc(sizeof(float) * 4);
  if (arr_5 == NULL) {
    fprintf(stderr, "malloc err\n");
    exit(1);
  }

  arr_5[0] = 11.0;
  arr_5[1] = 32.0;
  x4_pad(&arr_5, 2);
  assert(arr_5[0] == 11.0);
  assert(arr_5[1] == 32.0);
  assert(arr_5[2] == 0.0);
  assert(arr_5[3] == 0.0);

  free(arr_4);
  free(arr_5);
  return 0;
}

void x4_add(float **dest, float *arr_1, float *arr_2) {
  __m128 loaded1 = _mm_loadu_ps((const float *)arr_1);
  __m128 loaded2 = _mm_loadu_ps((const float *)arr_2);
  __m128 result = _mm_add_ps(loaded1, loaded2);
  _mm_store_ps(*dest, result);
}

void x4_pad(float **arr, int n) {}