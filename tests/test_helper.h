// SPDX-License-Identifier: GPL-2.0
#ifndef TEST_HELPER_H
#define TEST_HELPER_H
/*
 * Provides some common definitions and helper functions for the memory
 * tests.
 *
 * Copyright (C) 2020 Martin Whitaker.
 */

#include <stddef.h>
#include <stdint.h>

#include "test.h"

/*
 * A wrapper for guiding branch prediction.
 */
#define unlikely(x) __builtin_expect(!!(x), 0)

/*
 * The block size processed between each update of the progress bars and
 * spinners. This also affects how quickly the program will respond to the
 * keyboard.
 */
#define SPIN_SIZE (1 << 27)  // in testwords

/*
 * A macro to perform test bailout when requested.
 */
#define BAILOUT if (bail) return ticks

/*
 * Returns value rounded down to the nearest multiple of align_size.
 */
static inline uintptr_t round_down(uintptr_t value, size_t align_size)
{
    return value & ~(align_size - 1);
}

/*
 * Returns value rounded up to the nearest multiple of align_size.
 */
static inline uintptr_t round_up(uintptr_t value, size_t align_size)
{
    return (value + (align_size - 1)) & ~(align_size - 1);
}

/*
 * Seeds the psuedo-random number generator for my_vcpu.
 */
void random_seed(int my_vcpu, uint64_t seed);

/*
 * Returns a psuedo-random number for my_vcpu. The sequence of numbers returned
 * is repeatable for a given starting seed. The sequence repeats after 2^64 - 1
 * numbers. Within that period, no number is repeated.
 */
testword_t random(int my_vcpu);

/*
 * Calculates the start and end word address for the chunk of segment that is
 * to be tested by my_vcpu. The chunk start will be aligned to a multiple of
 * chunk_align.
 */
void calculate_chunk(testword_t **start, testword_t **end, int my_vcpu, int segment, size_t chunk_align);

#endif // TEST_HELPER_H
