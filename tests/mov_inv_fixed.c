// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2020 Martin Whitaker.
//
// Derived from an extract of memtest86+ test.c:
//
// MemTest86+ V5 Specific code (GPL V2.0)
// By Samuel DEMEULEMEESTER, sdemeule@memtest.org
// http://www.canardpc.com - http://www.memtest.org
// Thanks to Passmark for calculate_chunk() and various comments !
// ----------------------------------------------------
// test.c - MemTest-86  Version 3.4
//
// Released under version 2 of the Gnu Public License.
// By Chris Brady

#include <stdbool.h>
#include <stdint.h>

#include "display.h"
#include "error.h"
#include "test.h"

#include "test_funcs.h"
#include "test_helper.h"

#define HAND_OPTIMISED  1   // Use hand-optimised assembler code for performance.

//------------------------------------------------------------------------------
// Public Functions
//------------------------------------------------------------------------------

int test_mov_inv_fixed(int my_vcpu, int iterations, testword_t pattern1, testword_t pattern2)
{
    int ticks = 0;

    if (my_vcpu == master_vcpu) {
        display_test_pattern_value(pattern1);
    }

    // Initialize memory with the initial pattern.
    for (int i = 0; i < vm_map_size; i++) {
        testword_t *start, *end;
        calculate_chunk(&start, &end, my_vcpu, i, sizeof(testword_t));

        volatile testword_t *p  = start;
        volatile testword_t *pe = start;

        bool at_end = false;
        do {
            // take care to avoid pointer overflow
            if ((end - pe) >= SPIN_SIZE) {
                pe += SPIN_SIZE - 1;
            } else {
                at_end = true;
                pe = end;
            }
            ticks++;
            if (my_vcpu < 0) {
                continue;
            }
            test_addr[my_vcpu] = (uintptr_t)p;
#if HAND_OPTIMISED
#ifdef __x86_64__
            uint64_t length = pe - p + 1;
            __asm__  __volatile__ ("\t"
                "rep    \n\t"
                "stosq  \n\t"
                :
                : "c" (length), "D" (p), "a" (pattern1)
                :
            );
            p = pe;
#else
            uint32_t length = pe - p + 1;
            __asm__  __volatile__ ("\t"
                "rep    \n\t"
                "stosl  \n\t"
                :
                : "c" (length), "D" (p), "a" (pattern1)
                :
            );
            p = pe;
#endif
#else
            do {
                *p = pattern1;
            } while (p++ < pe); // test before increment in case pointer overflows
#endif
            do_tick(my_vcpu);
            BAILOUT;
        } while (!at_end && ++pe); // advance pe to next start point
    }

    // Check for the current pattern and then write the alternate pattern for
    // each memory location. Test from the bottom up and then from the top down.
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < vm_map_size; j++) {
            testword_t *start, *end;
            calculate_chunk(&start, &end, my_vcpu, j, sizeof(testword_t));

            volatile testword_t *p  = start;
            volatile testword_t *pe = start;

            bool at_end = false;
            do {
                // take care to avoid pointer overflow
                if ((end - pe) >= SPIN_SIZE) {
                    pe += SPIN_SIZE - 1;
                } else {
                    at_end = true;
                    pe = end;
                }
                ticks++;
                if (my_vcpu < 0) {
                    continue;
                }
                test_addr[my_vcpu] = (uintptr_t)p;
                do {
                    testword_t actual = *p;
                    if (unlikely(actual != pattern1)) {
                        data_error(p, pattern1, actual, true);
                    }
                    *p = pattern2;
                } while (p++ < pe); // test before increment in case pointer overflows
                do_tick(my_vcpu);
                BAILOUT;
            } while (!at_end && ++pe); // advance pe to next start point
        }

        for (int j = vm_map_size - 1; j >= 0; j--) {
            testword_t *start, *end;
            calculate_chunk(&start, &end, my_vcpu, j, sizeof(testword_t));

            volatile testword_t *p  = end;
            volatile testword_t *ps = end;

            bool at_start = false;
            do {
                // take care to avoid pointer underflow
                if ((ps - start) >= SPIN_SIZE) {
                    ps -= SPIN_SIZE - 1;
                } else {
                    at_start = true;
                    ps = start;
                }
                ticks++;
                if (my_vcpu < 0) {
                    continue;
                }
                test_addr[my_vcpu] = (uintptr_t)p;
                do {
                    testword_t actual = *p;
                    if (unlikely(actual != pattern2)) {
                        data_error(p, pattern2, actual, true);
                    }
                    *p = pattern1;
                } while (p-- > ps); // test before decrement in case pointer overflows
                do_tick(my_vcpu);
                BAILOUT;
            } while (!at_start && --ps); // advance ps to next start point
        }
    }

    return ticks;
}
