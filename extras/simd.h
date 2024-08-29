#include <immintrin.h>

// add 2 float arrays using intrinsic adds
void x4_add(float **dest, float *arr_1, float *arr_2);
// pad input arr with n zeros to the right
void x4_pad(float **arr, int n);
