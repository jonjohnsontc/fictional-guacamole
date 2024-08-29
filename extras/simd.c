#include <assert.h>
#include <immintrin.h>
#include <stdio.h>

/*
	Trying to get an understanding of which SIMD / instrinsic 
	functions are available for my system, and  how to best use
	them.
*/
int main(void) {
	float arr_1[] = { 2, 3, 7, 72 };
	float arr_2[] = { 8, 7, 2, 12 };

	__m128 loaded1 = _mm_loadu_ps((const float*)&arr_1);
	__m128 loaded2 = _mm_loadu_ps((const float*)&arr_2);
	__m128 result = _mm_add_ps(loaded1, loaded2);

	float arr_3[4];
	_mm_store_ps((float*)&arr_3, result);
	
	assert(arr_3[0] == 10.0);
	assert(arr_3[1] == 10.0);
	assert(arr_3[2] == 9.0);
	assert(arr_3[3] == 84.0);
	return 0;
}