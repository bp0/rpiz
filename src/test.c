/*
 *
 * Copyright (C) 2017  Burt P. (pburt0@gmail.com)
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
#include <stdio.h>
#include <string.h>
#include "board_rpi.h"
#include "cpu_arm.h"

static void dump_board(rpi_board *b) {
    if (b) {
        printf(
        "rpi_board_desc(): %s\n"
        "rpi_board_model(): %s\n"
        "rpi_board_rev(): %s\n"
        "rpi_board_intro(): %s\n"
        "rpi_board_mfgby(): %s\n"
        "rpi_board_rcode(): %s\n"
        "rpi_board_serial(): %s\n"
        "rpi_board_mem_spec(): %s\n"
        "rpi_board_soc: %s\n"
        "rpi_soc_temp(): %0.2f' C\n"
        "%s",
        rpi_board_desc(b),
        rpi_board_model(b),
        rpi_board_rev(b),
        rpi_board_intro(b),
        rpi_board_mfgby(b),
        rpi_board_rcode(b),
        rpi_board_serial(b),
        rpi_board_mem_spec(b),
        rpi_board_soc(b),
        rpi_soc_temp(),
        "");
    }
}

static void dump_proc(arm_proc *p) {
    int i;
    if (p) {
        /*
        printf(".proc.cpu_name = %s\n", p->cpu_name);
        printf(".proc.cpu_desc = %s\n", p->cpu_desc);
        printf(".proc.max_khz = %d\n", p->max_khz);
        printf(".proc.core_count = %d\n", p->core_count);
        for(i = 0; i < p->core_count; i++) {
            printf(".proc.core[%d].id = %d\n", i, p->cores[i].id);
            printf(".proc.core[%d].model_name = %s\n", i, p->cores[i].model_name);
            printf(".proc.core[%d].flags = %s\n", i, p->cores[i].flags);
            printf(".proc.core[%d].cpu_implementer = %s\n", i, p->cores[i].cpu_implementer);
            printf(".proc.core[%d].cpu_architecture = %s\n", i, p->cores[i].cpu_architecture);
            printf(".proc.core[%d].cpu_variant = %s\n", i, p->cores[i].cpu_variant);
            printf(".proc.core[%d].cpu_part = %s\n", i, p->cores[i].cpu_part);
            printf(".proc.core[%d].cpu_revision = %s\n", i, p->cores[i].cpu_revision);
            printf(".proc.core[%d].freq_khz(min - max / cur) = %d - %d / %d\n", i, 
                p->cores[i].cpukhz_min, p->cores[i].cpukhz_max, p->cores[i].cpukhz_cur );
        }
        */
    }
    printf("arm_flag_list() = %s\n", arm_flag_list() );
}

int main(void) {
    rpi_board *b;
    arm_proc *p;

    b = rpi_board_new();
    p = arm_proc_new();
    if (p == NULL) {
        printf("Scan CPU failed.\n");
        return 1;
    }
    dump_board(b);
    dump_proc(p);
    arm_proc_free(p);
    rpi_board_free(b);
    return 0;
}
