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
#include "util.h"
#include "cpu_arm.h"

#define MAX_CORES 8

static int search_for_flag(char *flags, const char *flag) {
    char *p = strstr(flags, flag);
    int l = strlen(flag);
    int front = 0, back = 0;
    //DEBUG printf("search_for_flag( %x, \"%s\")\n", flags, flag);
    if (strlen(flag) == 0 || strchr(flag, ' ') )
        return 0;
    while (p) {
        if (p == flags) front = 1;
        else if (*(p - 1) == ' ') front = 1;
        if (*(p + l) == ' ' || *(p + l) == '\0')
            back = 1;
        if (front && back)
            return 1;
        p = strstr(p + l, flag);
    }
    return 0;
}

#define APPEND_FLAG(f) strcat(all_flags, f); strcat(all_flags, " ");
static void add_unknown_flags(char *flags_str) {
    char flag[16] = "";
    char *all_flags; /* arm_data.c: static char all_flags[1024] */
    char *cur, *next;
    int added_count = 0;
    if (flags_str) {
        all_flags = (char*)arm_flag_list();
        cur = flags_str;
        next = strchr(cur, ' '); if (!next) next = strchr(cur, '\0');
        while(next) {
            if (next-cur <= 15) {
                memset(flag, 0, 16);
                strncpy(flag, cur, next-cur);
                if (strlen(flag) > 0) {
                    if (!search_for_flag(all_flags, flag)) {
                        APPEND_FLAG(flag);
                        added_count++;
                    }
                }
            }
            if (*next == '\0') break;
            cur = next + 1;
            next = strchr(cur, ' '); if (!next) next = strchr(cur, '\0');
        }
    }
    // DEBUG printf("add_unknown_flags(): added %d previously unknown flags\n", added_count);
}

typedef struct {
    int ref_count;
    char *str;
} cpu_string;

typedef struct {
    int count;
    cpu_string *strs;
} cpu_string_list;

typedef struct {
    int id;
    int cpukhz_min, cpukhz_max, cpukhz_cur;

    /* point to a cpu_string.str */
    char *model_name;
    char *flags;
    char *cpu_implementer;
    char *cpu_architecture;
    char *cpu_variant;
    char *cpu_part;
    char *cpu_revision;
    char *decoded_name;
    char *cpukhz_max_str;
} arm_core;

struct arm_proc {
    cpu_string_list *model_name;
    cpu_string_list *decoded_name;
    cpu_string_list *flags;
    cpu_string_list *cpu_implementer;
    cpu_string_list *cpu_architecture;
    cpu_string_list *cpu_variant;
    cpu_string_list *cpu_part;
    cpu_string_list *cpu_revision;
    cpu_string_list *cpukhz_max_str;
    
    char cpu_name[256];
    char *cpu_desc;
    int max_khz;
    int core_count;
    arm_core cores[MAX_CORES];
};

static int get_cpu_int(const char* file, int cpuid) {
    char fn[256];
    char *fc = NULL;
    int ret = 0;
    //DEBUG printf("get_cpu_int( %s , %d )\n", file, cpuid);
    snprintf(fn, 256, "/sys/devices/system/cpu/cpu%d/%s", cpuid, file);
    fc = get_file_contents(fn);
    if (fc) {
        ret = atol(fc);
        free(fc);
    }
    return ret;
}

static cpu_string_list *strlist_new(void) {
    cpu_string_list *list = malloc( sizeof(cpu_string_list) );
    list->count = 0;
    list->strs = NULL;
    return list;
}

static void strlist_free(cpu_string_list *list) {
    int i;
    for (i = 0; i < list->count; i++) {
        free(list->strs[i].str);
    }
    free(list);
    list = NULL;
}

static char *strlist_add(cpu_string_list *list, const char* str) {
    int i;
    for (i = 0; i < list->count; i++) {
        if (strcmp(list->strs[i].str, str) == 0) {
            /* found */
            list->strs[i].ref_count++;
            return list->strs[i].str;
        }
    }
    /* not found */
    i = list->count; list->count++;
    
    if (list->strs == NULL)
        list->strs = malloc(sizeof(cpu_string));
    else
        list->strs = realloc(list->strs, sizeof(cpu_string) * list->count);
    
    list->strs[i].str = malloc(strlen(str) + 1);
    strcpy(list->strs[i].str, str);
    list->strs[i].ref_count = 1;
    return list->strs[i].str;
}

#define CHECK_FOR(k) (strncmp(k, key, (strlen(k) < strlen(key)) ? strlen(k) : strlen(key)) == 0)
#define GET_STR(k, s) if (CHECK_FOR(k)) { p->cores[core].s = strlist_add(p->s, value); continue; }
#define FIN_PROC() if (core >= 0) if (!p->cores[core].model_name) { p->cores[core].model_name = strlist_add(p->model_name, rep_pname); }

#define REDUP(f) if(p->cores[di].f) { p->cores[i].f = strlist_add(p->f, p->cores[di].f); }

#ifndef PROC_CPUINFO
#define PROC_CPUINFO "/proc/cpuinfo"
#endif

static int scan_cpu(arm_proc* p) {
    kv_scan *kv; char *key, *value;
    int core = -1;
    int i, di;
    char rep_pname[256] = "";
    char tmp_maxfreq[128];
    char *tmp_dn = NULL;

    if (!p) return 0;

    kv = kv_new_file(PROC_CPUINFO);
    if (kv) {
        while( kv_next(kv, &key, &value) ) {
            if (CHECK_FOR("Processor")) {
                strcpy(rep_pname, value);
                continue;
            }

            if (CHECK_FOR("Hardware")) {
                strcpy(p->cpu_name, value);
                continue;
            }

            if (CHECK_FOR("processor")) {
                FIN_PROC();
                core++;
                memset(&p->cores[core], 0, sizeof(arm_core));
                p->cores[core].id = atoi(value);
                continue;
            }

            if (core < 0) {
                if ( CHECK_FOR("model name")
                     || CHECK_FOR("Features")
                     || CHECK_FOR("flags") ) {
                    /* this cpuinfo doesn't provide processor : n
                     * there is prolly only one core */
                    core++;
                    memset(&p->cores[core], 0, sizeof(arm_core));
                    p->cores[core].id = 0;
                }
            }
            if (core >= 0) {
                GET_STR("model name", model_name);

                /* likely one or the other */
                GET_STR("Features", flags);
                GET_STR("flags", flags);

                /* ARM */
                GET_STR("CPU implementer", cpu_implementer);
                GET_STR("CPU architecture", cpu_architecture);
                GET_STR("CPU variant", cpu_variant);
                GET_STR("CPU part", cpu_part);
                GET_STR("CPU revision", cpu_revision);
            }
        }
        FIN_PROC();
        kv_free(kv);
    } else
        return 0;

    p->core_count = core + 1;

    /* re-duplicate missing data for /proc/cpuinfo variant that de-duplicated it */
    di = p->core_count - 1;
    for (i = di; i >= 0; i--) {
        if (p->cores[i].flags)
            di = i;
        else {
            REDUP(flags);
            REDUP(cpu_implementer);
            REDUP(cpu_architecture);
            REDUP(cpu_variant);
            REDUP(cpu_part);
            REDUP(cpu_revision);
        }
    }

    /* data not from /proc/cpuinfo */
    for (i = 0; i < p->core_count; i++) {
        /* decoded names */
        tmp_dn = arm_decoded_name(
                p->cores[i].cpu_implementer, p->cores[i].cpu_architecture,
                p->cores[i].cpu_part, p->cores[i].cpu_variant, p->cores[i].cpu_revision,
                p->cores[i].model_name);
        p->cores[i].decoded_name = strlist_add(p->decoded_name, tmp_dn);
        free(tmp_dn); tmp_dn = NULL;

        /* freq */
        p->cores[i].cpukhz_cur = get_cpu_int("cpufreq/scaling_cur_freq", p->cores[i].id);
        p->cores[i].cpukhz_min = get_cpu_int("cpufreq/scaling_min_freq", p->cores[i].id);
        p->cores[i].cpukhz_max = get_cpu_int("cpufreq/scaling_max_freq", p->cores[i].id);
        sprintf(tmp_maxfreq, "%d", p->cores[i].cpukhz_max);
        p->cores[i].cpukhz_max_str = strlist_add(p->cpukhz_max_str, tmp_maxfreq);
        if (p->cores[i].cpukhz_max > p->max_khz)
            p->max_khz = p->cores[i].cpukhz_max;
    }

    return 1;
}

static char *gen_cpu_desc(arm_proc *p) {
    char *ret = NULL;
    char tmp[1024];
    int i, l = 0;
    float maxfreq;
    if (p) {
        ret = malloc(4096);
        memset(ret, 0, 4096);
        for (i = 0; i < p->decoded_name->count; i++) {
            sprintf(tmp, "%dx %s", p->decoded_name->strs[i].ref_count, p->decoded_name->strs[i].str);
            sprintf(ret + l, "%s%s", (i>0) ? " + " : "", tmp);
            l += (i>0) ? strlen(tmp) + 3 : strlen(tmp);
        }
        sprintf(ret + l, "; "); l += 2;
        for (i = 0; i < p->cpukhz_max_str->count; i++) {
            maxfreq = atof(p->cpukhz_max_str->strs[i].str);
            if (maxfreq)
                maxfreq /= 1000;
            else
                maxfreq = 0.0f;
            sprintf(tmp, "%dx %0.2f MHz", p->cpukhz_max_str->strs[i].ref_count, maxfreq);
            sprintf(ret + l, "%s%s", (i>0) ? " + " : "", tmp);
            l += (i>0) ? strlen(tmp) + 3 : strlen(tmp);
        }        
    }
    return ret;
}

static void scan_for_unknown_flags(arm_proc *s) {
    int i;
    if (s) {
        for(i = 0; i < s->flags->count; i++)
            add_unknown_flags(s->flags->strs[i].str);
    }
}

arm_proc *arm_proc_new(void) {
    arm_proc *s = malloc( sizeof(arm_proc) );
    if (s) {
        memset(s, 0, sizeof(*s));
        s->model_name = strlist_new();
        s->flags = strlist_new();
        s->cpu_implementer = strlist_new();
        s->cpu_architecture = strlist_new();
        s->cpu_variant = strlist_new();
        s->cpu_part = strlist_new();
        s->cpu_revision = strlist_new();
        s->decoded_name = strlist_new();
        s->cpukhz_max_str = strlist_new();
        if (!scan_cpu(s)) {
            arm_proc_free(s);
            return NULL;
        }
        s->cpu_desc = gen_cpu_desc(s);
        scan_for_unknown_flags(s);
    }
    return s;
}

void arm_proc_free(arm_proc *s) {
    if(s) {
        strlist_free(s->model_name);
        strlist_free(s->flags);
        strlist_free(s->cpu_implementer);
        strlist_free(s->cpu_architecture);
        strlist_free(s->cpu_variant);
        strlist_free(s->cpu_part);
        strlist_free(s->cpu_revision);
        strlist_free(s->decoded_name);
        strlist_free(s->cpukhz_max_str);
        free(s->cpu_desc);
        free(s);
    }
}

const char *arm_proc_name(arm_proc *s) {
    if (s)
        return s->cpu_name;
    else
        return NULL;
}

const char *arm_proc_desc(arm_proc *s) {
    if (s)
        return s->cpu_desc;
    else
        return NULL;
}

int arm_proc_has_flag(arm_proc *s, const char *flag) {
    int i;
    if (s) {
        for (i = 0; i < s->flags->count; i++)
            if (search_for_flag(s->flags->strs[i].str, flag))
                return 1;
    }
    return 0;
}

int arm_proc_cores(arm_proc *s) {
    if (s)
        return s->core_count;
    else
        return 0;
}

int arm_proc_core_from_id(arm_proc *s, int id) {
    int i = 0;
    if (s)
        for (i = 0; i < s->core_count; i++ )
            if (s->cores[i].id == id)
                return i;

    return -1;
}

int arm_proc_core_id(arm_proc *s, int core) {
    if (s)
        if (core >= 0 && core < s->core_count)
            return s->cores[core].id;

    return 0;
}

int arm_proc_core_khz_min(arm_proc *s, int core) {
    if (s)
        if (core >= 0 && core < s->core_count)
            return s->cores[core].cpukhz_min;

    return 0;
}

int arm_proc_core_khz_max(arm_proc *s, int core) {
    if (s)
        if (core >= 0 && core < s->core_count)
            return s->cores[core].cpukhz_max;

    return 0;
}

int arm_proc_core_khz_cur(arm_proc *s, int core) {
    if (s)
        if (core >= 0 && core < s->core_count) {
            s->cores[core].cpukhz_cur = get_cpu_int("cpufreq/scaling_cur_freq", s->cores[core].id);
            return s->cores[core].cpukhz_cur;
        }
    return 0;
}

#ifdef DEBUG_ARMCPU

static void dump(arm_proc *p) {
    int i;
    if (p) {
        printf(".proc.cpu_name = %s\n", p->cpu_name);
        printf(".proc.cpu_desc = %s\n", p->cpu_desc);
        printf(".proc.max_khz = %d\n", p->max_khz);
        printf(".proc.core_count = %d\n", p->core_count);
        for(i = 0; i < p->core_count; i++) {
            printf(".proc.core[%d].id = %d\n", i, p->cores[i].id);
            printf(".proc.core[%d].model_name = %s\n", i, p->cores[i].model_name);
            printf(".proc.core[%d].decoded_name = %s\n", i, p->cores[i].decoded_name);
            printf(".proc.core[%d].flags = %s\n", i, p->cores[i].flags);
            printf(".proc.core[%d].cpu_implementer = [%s] %s\n", i, p->cores[i].cpu_implementer, arm_implementer(p->cores[i].cpu_implementer) );
            printf(".proc.core[%d].cpu_architecture = %s\n", i, p->cores[i].cpu_architecture);
            printf(".proc.core[%d].cpu_variant = %s\n", i, p->cores[i].cpu_variant);
            printf(".proc.core[%d].cpu_part = [%s] %s\n", i, p->cores[i].cpu_part, arm_arm_part(p->cores[i].cpu_part) );
            printf(".proc.core[%d].cpu_revision = %s\n", i, p->cores[i].cpu_revision);
            printf(".proc.core[%d].freq_khz(min - max / cur) = %d - %d / %d\n", i, 
                p->cores[i].cpukhz_min, p->cores[i].cpukhz_max, p->cores[i].cpukhz_cur );
        }
    }
    printf(".all_flags = %s (len: %d)\n", arm_flag_list(), strlen( arm_flag_list() ) );
}

int main(void) {
    arm_proc *p;
    
    p = arm_proc_new();
    if (p == NULL) {
        printf("Scan CPU failed.\n");
        return 1;
    }
    dump(p);
    arm_proc_free(p);
    return 0;
}

#endif
