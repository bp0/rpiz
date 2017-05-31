/*
 * rpiz - https://github.com/bp0/rpiz
 * Copyright (C) 2017  Burt P. <pburt0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <stdlib.h>
#include "cpu.h"
#include "cpu_arm.h"
#include "arm_data.h"
#include "cpu_x86.h"
#include "x86_data.h"

typedef enum {
    PT_UNKNOWN = 0,
    PT_ARM,
    PT_X86,
    PT_N_TYPES,
} cpu_type;

struct {
    cpu_type type;
    union {
        arm_proc *arm;
        x86_proc *x86;
    };
} cpu;

int cpu_init() {
    cpu.type = PT_UNKNOWN;

#if defined(__arm__) || defined(__aarch64__)
    cpu.arm = arm_proc_new();
    cpu.type = PT_ARM;
#endif

#if defined(__i386__) || defined(__x86_64__)
    cpu.x86 = x86_proc_new();
    cpu.type = PT_X86;
#endif

    return 1;
}

void cpu_cleanup() {
    switch (cpu.type) {
        case (PT_ARM):
            if (cpu.arm) arm_proc_free(cpu.arm);
            break;
        case (PT_X86):
            if (cpu.x86) x86_proc_free(cpu.x86);
            break;
        default:
            break;
    }
}

const char *cpu_all_flags(void) {
    switch (cpu.type) {
        case (PT_ARM):
            return arm_flag_list();
        case (PT_X86):
            return x86_flag_list();
        default:
            return NULL;
    }
}

int cpu_has_flag(const char *flag) {
    switch (cpu.type) {
        case (PT_ARM):
            return arm_proc_has_flag(cpu.arm, flag);
        case (PT_X86):
            return x86_proc_has_flag(cpu.x86, flag);
        default:
            return 0;
    }
}

const char *cpu_flag_meaning(const char *flag) {
    switch (cpu.type) {
        case (PT_ARM):
            return arm_flag_meaning(flag);
        case (PT_X86):
            return x86_flag_meaning(flag);
        default:
            return NULL;
    }
}

rpiz_fields *cpu_fields() {
    switch (cpu.type) {
        case (PT_ARM):
            return arm_proc_fields(cpu.arm);
        case (PT_X86):
            return x86_proc_fields(cpu.x86);
        default:
            return NULL;
    }
    return NULL;
}
