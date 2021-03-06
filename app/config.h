// SPDX-License-Identifier: GPL-2.0
#ifndef CONFIG_H
#define CONFIG_H
/*
 * Provides the configuration settings and pop-up menu.
 *
 * Copyright (C) 2020 Martin Whitaker.
 */

#include <stdbool.h>
#include <stdint.h>

#include "smp.h"

typedef enum {
    PAR,
    SEQ,
    ONE
} cpu_mode_t;

typedef enum {
    ERROR_MODE_NONE,
    ERROR_MODE_SUMMARY,
    ERROR_MODE_ADDRESS,
    ERROR_MODE_BADRAM
} error_mode_t;

extern uintptr_t    pm_limit_lower;
extern uintptr_t    pm_limit_upper;

extern uintptr_t    num_pages_to_test;

extern cpu_mode_t   cpu_mode;

extern error_mode_t error_mode;

extern bool         enable_pcpu[MAX_PCPUS];

extern bool         enable_temperature;
extern bool         enable_trace;

void config_init(void);

void config_menu(bool initial);

void initial_config(void);

#endif // CONFIG_H
