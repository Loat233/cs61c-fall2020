#include <time.h>
#include <stdio.h>
#include <x86intrin.h>
#include "simd.h"

long long int sum(int vals[NUM_ELEMS]) {
	clock_t start = clock();

	long long int sum = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS; i++) {
			if(vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	long long int sum = 0;

	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
			if(vals[i] >= 128) sum += vals[i];
			if(vals[i + 1] >= 128) sum += vals[i + 1];
			if(vals[i + 2] >= 128) sum += vals[i + 2];
			if(vals[i + 3] >= 128) sum += vals[i + 3];
		}

		//This is what we call the TAIL CASE
		//For when NUM_ELEMS isn't a multiple of 4
		//NONTRIVIAL FACT: NUM_ELEMS / 4 * 4 is the largest multiple of 4 less than NUM_ELEMS
		for(unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_simd(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);		// This is a vector with 127s in it... Why might you need this?
	long long int result = 0;				   // This is where you should put your final result!
	/* DO NOT DO NOT DO NOT DO NOT WRITE ANYTHING ABOVE THIS LINE. */
	
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		__m128i sum_vec = _mm_setzero_si128();
		unsigned int i;
		for(i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
			__m128i cur_vals = _mm_loadu_si128((__m128i*) (vals + i));
			__m128i mask = _mm_cmpgt_epi32(cur_vals, _127);
			__m128i fil_vals = _mm_and_si128(cur_vals, mask);
			sum_vec = _mm_add_epi32(fil_vals, sum_vec);
		}

		int tmp[4];
		_mm_storeu_si128((__m128i*) tmp, sum_vec);
		result += tmp[0] + tmp[1] + tmp[2] + tmp[3];

		for(; i < NUM_ELEMS; i++) {
			if (vals[i] > 127) {
				result += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}

long long int sum_simd_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);
	long long int result = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		__m128i sum_vec0 = _mm_setzero_si128();
		__m128i sum_vec1 = _mm_setzero_si128();
		__m128i sum_vec2 = _mm_setzero_si128();
		__m128i sum_vec3 = _mm_setzero_si128();

		unsigned int i;
		for(i = 0; i < NUM_ELEMS / 16 * 16; i += 16) {
			__m128i vals0 = _mm_loadu_si128((__m128i*) (vals + i));
			__m128i mask0 = _mm_cmpgt_epi32(vals0, _127);
			vals0 = _mm_and_si128(vals0, mask0);
			sum_vec0 = _mm_add_epi32(vals0, sum_vec0);

			__m128i vals1 = _mm_loadu_si128((__m128i*) (vals + i + 4));
			__m128i mask1 = _mm_cmpgt_epi32(vals1, _127);
			vals1 = _mm_and_si128(vals1, mask1);
			sum_vec1 = _mm_add_epi32(vals1, sum_vec1);

			__m128i vals2 = _mm_loadu_si128((__m128i*) (vals + i + 8));
			__m128i mask2 = _mm_cmpgt_epi32(vals2, _127);
			vals2 = _mm_and_si128(vals2, mask2);
			sum_vec2 = _mm_add_epi32(vals2, sum_vec2);

			__m128i vals3 = _mm_loadu_si128((__m128i*) (vals + i + 12));
			__m128i mask3 = _mm_cmpgt_epi32(vals3, _127);
			vals3 = _mm_and_si128(vals3, mask3);
			sum_vec3 = _mm_add_epi32(vals3, sum_vec3);
		}

		sum_vec0 = _mm_add_epi32(sum_vec0, sum_vec1);
		sum_vec2 = _mm_add_epi32(sum_vec2, sum_vec3);
		sum_vec0 = _mm_add_epi32(sum_vec0, sum_vec2);


		int tmp[4];
		_mm_storeu_si128((__m128i*) tmp, sum_vec0);
		result += tmp[0] + tmp[1] + tmp[2] + tmp[3];

		for(; i < NUM_ELEMS; i++) {
			if (vals[i] > 127) {
				result += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}