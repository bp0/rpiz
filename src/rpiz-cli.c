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
#include <stdio.h>
#include <string.h>
#include "board_rpi.h"
#include "cpu_arm.h"

static void dump_fields(rpiz_fields *f) {
    char *t, *n, *v;
    while (f) {
        fields_get(f, &t, &n, &v);
        printf("[%s] %s = %s\n", t, n, v);
        f = fields_next(f);
    }
}

static void dump_proc(arm_proc *p) {
    /* int i; */
    if (p) {
        printf("arm_proc_name(): %s\n", arm_proc_name(p) );
        printf("arm_proc_desc(): %s\n", arm_proc_desc(p) );
        printf("arm_proc_cores(): %d\n", arm_proc_cores(p) );
        /*
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
    printf("arm_flag_list() = %s (len:%d)\n", arm_flag_list(), strlen(arm_flag_list()) );
}

int main(void) {
    rpi_board *b;
    arm_proc *p;
    rpiz_fields *bf, *pf;

    b = rpi_board_new();
    p = arm_proc_new();
    if (b == NULL) {
        printf("Scan board failed.\n");
        return 1;
    }
    if (p == NULL) {
        printf("Scan proc failed.\n");
        return 1;
    }
    //dump_proc(p);

    bf = rpi_board_fields(b);
    dump_fields(bf);
    pf = arm_proc_fields(p);
    dump_fields(pf);

    arm_proc_free(p);
    rpi_board_free(b);
    return 0;
}
