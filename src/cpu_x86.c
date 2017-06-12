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
#include "util.h"
#include "cpu_x86.h"

#define MAX_THREADS 128

static const char unk[] = "";

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

typedef struct {
    int id, core, proc;
    int cpukhz_min, cpukhz_max, cpukhz_cur;

    /* point to a cpu_string.str */
    char *model_name;
    char *decoded_name;
    char *flags;
    char *bug_flags;
    char *pm_flags;
    char *cpukhz_max_str;

    char *physical_id;
    char *core_id;

    int bug_fdiv, bug_hlt, bug_f00f, bug_coma;
} x86_thread;

struct x86_proc {
    cpu_string_list *model_name;
    cpu_string_list *decoded_name;
    cpu_string_list *flags;
    cpu_string_list *bug_flags;
    cpu_string_list *pm_flags;
    cpu_string_list *cpukhz_max_str;

    cpu_string_list *physical_id;
    cpu_string_list *core_id;

    cpu_string_list *each_flag;

    char *cpu_name; /* do not free */
    char *cpu_desc;
    int max_khz;

    int thread_count;
    x86_thread threads[MAX_THREADS];
    int core_count;
    int proc_count;

    rpiz_fields *fields;
};

#define CHECK_FOR(k) (strncmp(k, key, (strlen(k) < strlen(key)) ? strlen(k) : strlen(key)) == 0)
#define GET_STR(k, s) if (CHECK_FOR(k)) { p->threads[thread].s = strlist_add(p->s, value); continue; }
#define FIN_PROC() if (thread >= 0) if (!p->threads[thread].model_name) { p->threads[thread].model_name = strlist_add(p->model_name, rep_pname); }

#define REDUP(f) if(p->threads[di].f && !p->threads[i].f) { p->threads[i].f = strlist_add(p->f, p->threads[di].f); }

#ifndef PROC_CPUINFO
#define PROC_CPUINFO "/proc/cpuinfo"
#endif

static int scan_cpu(x86_proc* p) {
    kv_scan *kv; char *key, *value;
    int thread = -1;
    int i, di;
    char rep_pname[256] = "";
    char tmp_maxfreq[128];
    char *tmp_str = NULL;

    if (!p) return 0;

    kv = kv_new_file(PROC_CPUINFO);
    if (kv) {
        while( kv_next(kv, &key, &value) ) {
            if (CHECK_FOR("Processor")) {
                strcpy(rep_pname, value);
                continue;
            }

            if (CHECK_FOR("processor")) {
                FIN_PROC();
                thread++;
                memset(&p->threads[thread], 0, sizeof(x86_thread));
                p->threads[thread].id = atoi(value);
                continue;
            }

            if (thread < 0) {
                if ( CHECK_FOR("model name")
                     || CHECK_FOR("flags") ) {
                    /* this cpuinfo doesn't provide processor : n
                     * there is prolly only one thread */
                    thread++;
                    memset(&p->threads[thread], 0, sizeof(x86_thread));
                    p->threads[thread].id = 0;
                }
            }
            if (thread >= 0) {
                GET_STR("model name", model_name);

                GET_STR("physical id", physical_id);
                GET_STR("core id", core_id);

                GET_STR("flags", flags);
                GET_STR("bugs", bug_flags);
                GET_STR("power management", pm_flags);

                if (CHECK_FOR("fdiv_bug") ) {
                    if (strncmp(value, "yes", 3) == 0)
                        p->threads[thread].bug_fdiv = 1;
                }
                if (CHECK_FOR("hlt_bug")) {
                    if (strncmp(value, "yes", 3) == 0)
                        p->threads[thread].bug_hlt = 1;
                }
                if (CHECK_FOR("f00f_bug")) {
                    if (strncmp(value, "yes", 3) == 0)
                        p->threads[thread].bug_f00f = 1;
                }
                if (CHECK_FOR("coma_bug")) {
                    if (strncmp(value, "yes", 3) == 0)
                        p->threads[thread].bug_coma = 1;
                }

            }
        }
        FIN_PROC();
        kv_free(kv);
    } else
        return 0;

    p->thread_count = thread + 1;

    /* re-duplicate missing data for /proc/cpuinfo variant that de-duplicated it */
    di = p->thread_count - 1;
    for (i = di; i >= 0; i--) {
        if (p->threads[i].flags)
            di = i;
        else {
            REDUP(flags);
            REDUP(bug_flags);
            REDUP(pm_flags);
        }
    }

    /* thread/core stuff */
    for (i = 0; i < p->thread_count; i++) {
        if (p->threads[i].core_id)
            p->threads[i].core = strtol(p->threads[i].core_id, NULL, 0);
        else
            p->threads[i].core = p->threads[i].id;

        if (p->threads[i].physical_id)
            p->threads[i].proc = strtol(p->threads[i].physical_id, NULL, 0);
    }
    p->core_count = p->core_id->count;
    p->proc_count = p->physical_id->count;
    if (!p->core_count) p->core_count = p->thread_count;
    if (!p->proc_count) p->proc_count = p->thread_count;

    /* data not from /proc/cpuinfo */
    for (i = 0; i < p->thread_count; i++) {
        if (p->threads[i].bug_flags == NULL) {
            /* make bugs list on old kernels that don't offer one */
            tmp_str = malloc(128);
            if (tmp_str) {
                memset(tmp_str, 0, 128);
                snprintf(tmp_str, 127, "%s%s%s%s%s%s%s%s%s%s",
                    p->threads[i].bug_fdiv ? " fdiv" : "",
                    p->threads[i].bug_hlt  ? " _hlt" : "",
                    p->threads[i].bug_f00f ? " f00f" : "",
                    p->threads[i].bug_coma ? " coma" : "",
                    /* these bug workarounds were reported as "features" in older kernels */
                    search_for_flag(p->threads[i].flags, "fxsave_leak")     ? " fxsave_leak" : "",
                    search_for_flag(p->threads[i].flags, "clflush_monitor") ? " clflush_monitor" : "",
                    search_for_flag(p->threads[i].flags, "11ap")            ? " 11ap" : "",
                    search_for_flag(p->threads[i].flags, "tlb_mmatch")      ? " tlb_mmatch" : "",
                    search_for_flag(p->threads[i].flags, "apic_c1e")        ? " apic_c1e" : "",
                    ""); /* just to make adding lines easier */
                if (strlen(tmp_str) > 0)
                    p->threads[i].bug_flags = strlist_add(p->bug_flags, tmp_str + 1); /* skip the first space */
                free(tmp_str); tmp_str = NULL;
            }
        }

        /* decoded names */
        tmp_str = strdup("(Unknown)");
        p->threads[i].decoded_name = strlist_add(p->decoded_name, tmp_str);
        free(tmp_str); tmp_str = NULL;

        /* freq */
        get_cpu_freq(p->threads[i].id, &p->threads[i].cpukhz_min, &p->threads[i].cpukhz_max, &p->threads[i].cpukhz_cur);
        sprintf(tmp_maxfreq, "%d", p->threads[i].cpukhz_max);
        p->threads[i].cpukhz_max_str = strlist_add(p->cpukhz_max_str, tmp_maxfreq);
        if (p->threads[i].cpukhz_max > p->max_khz)
            p->max_khz = p->threads[i].cpukhz_max;
    }

    return 1;
}

static char *gen_cpu_desc(x86_proc *p) {
    char *ret = NULL;
    char tmp[1024];
    int i, l = 0;
    float maxfreq;
    if (p) {
        ret = malloc(4096);
        memset(ret, 0, 4096);
        for (i = 0; i < p->model_name->count; i++) {
            if (p->model_name->count > 1)
                sprintf(tmp, "%dx %s", p->model_name->strs[i].ref_count, p->model_name->strs[i].str);
            else
                sprintf(tmp, "%s", p->model_name->strs[i].str);

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

#define APPEND_FLAG(f) strcat(all_flags, f); strcat(all_flags, " ");
static void process_flags(x86_proc *s) {
    char flag[32] = "";
    char *all_flags; /* x86_data.c: static char all_flags[4096] */
    char *cur, *next;
    int added_count = 0, i, si;
    if (!s) return;

    cpu_string_list *sets[3] = { s->flags, s->bug_flags, s->pm_flags };
    char *prefix[3] = { "", "bug:", "pm:" };
    int plen = 0, flen = 0;

    all_flags = (char*)x86_flag_list();

    for(si = 0; si < 3; si++) {
        plen = strlen(prefix[si]);
        for(i = 0; i < sets[si]->count; i++) {
            if (sets[si]->strs[i].str) {
                cur = sets[si]->strs[i].str;
                next = strchr(cur, ' '); if (!next) next = strchr(cur, '\0');
                while(next) {
                    flen = next-cur;
                    if (flen <= (31 - plen) ) {
                        memset(flag, 0, 32);
                        snprintf(flag, plen + flen + 1, "%s%s", prefix[si], cur);
                        if (strlen(flag) > (unsigned int)plen) {
                            /* add it to the string list, copy the flag string's ref_count */
                            strlist_add_w(s->each_flag, flag, sets[si]->strs[i].ref_count);

                            /* add it to the list of known all flags, if it isn't there */
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
        }
    }
    //DEBUG printf("process_flags(): added %d previously unknown flags\n(%d): %s\n", added_count, (int)strlen(all_flags), all_flags );
}

x86_proc *x86_proc_new(void) {
    x86_proc *s = malloc( sizeof(x86_proc) );
    if (s) {
        memset(s, 0, sizeof(*s));
        s->model_name = strlist_new();
        s->decoded_name = strlist_new();
        s->flags = strlist_new();
        s->bug_flags = strlist_new();
        s->pm_flags = strlist_new();
        s->cpukhz_max_str = strlist_new();
        s->core_id = strlist_new();
        s->physical_id = strlist_new();
        s->each_flag = strlist_new();
        if (!scan_cpu(s)) {
            x86_proc_free(s);
            return NULL;
        }
        s->cpu_desc = gen_cpu_desc(s);
        if (s->model_name->count == 1)
            s->cpu_name = s->model_name->strs[0].str;
        else
            s->cpu_name = (char *)unk;
        process_flags(s);
    }
    return s;
}

void x86_proc_free(x86_proc *s) {
    if(s) {
        strlist_free(s->model_name);
        strlist_free(s->decoded_name);
        strlist_free(s->flags);
        strlist_free(s->bug_flags);
        strlist_free(s->pm_flags);
        strlist_free(s->cpukhz_max_str);
        strlist_free(s->core_id);
        strlist_free(s->physical_id);
        strlist_free(s->each_flag);
        fields_free(s->fields);
        free(s->cpu_desc);
        free(s);
    }
}

const char *x86_proc_name(x86_proc *s) {
    if (s)
        return s->cpu_name;
    else
        return NULL;
}

const char *x86_proc_desc(x86_proc *s) {
    if (s)
        return s->cpu_desc;
    else
        return NULL;
}

int x86_proc_has_flag(x86_proc *s, const char *flag) {
    int i;
    if (s && flag) {
        for (i = 0; i < s->each_flag->count; i++) {
            //DEBUG printf("(%s)...[%d/%d] %s %d\n", flag, i, s->each_flag->count, s->each_flag->strs[i].str, s->each_flag->strs[i].ref_count);
            if (strcmp(s->each_flag->strs[i].str, flag) == 0)
                return s->each_flag->strs[i].ref_count;
        }
    }
    return 0;
}

int x86_proc_threads(x86_proc *s) {
    if (s)
        return s->thread_count;
    else
        return 0;
}

int x86_proc_cores(x86_proc *s) {
    if (s)
        return s->core_count;
    else
        return 0;
}

int x86_proc_count(x86_proc *s) {
    if (s)
        return s->proc_count;
    else
        return 0;
}

int x86_proc_thread_from_id(x86_proc *s, int id) {
    int i = 0;
    if (s)
        for (i = 0; i < s->thread_count; i++ )
            if (s->threads[i].id == id)
                return i;

    return -1;
}

int x86_proc_thread_id(x86_proc *s, int thread) {
    if (s)
        if (thread >= 0 && thread < s->thread_count)
            return s->threads[thread].id;

    return 0;
}

int x86_proc_thread_khz_min(x86_proc *s, int thread) {
    if (s)
        if (thread >= 0 && thread < s->thread_count)
            return s->threads[thread].cpukhz_min;

    return 0;
}

int x86_proc_thread_khz_max(x86_proc *s, int thread) {
    if (s)
        if (thread >= 0 && thread < s->thread_count)
            return s->threads[thread].cpukhz_max;

    return 0;
}

int x86_proc_thread_khz_cur(x86_proc *s, int thread) {
    if (s)
        if (thread >= 0 && thread < s->thread_count) {
            get_cpu_freq(s->threads[thread].id, NULL, NULL, &s->threads[thread].cpukhz_cur);
            return s->threads[thread].cpukhz_cur;
        }
    return 0;
}

static char* x86_proc_threads_str(x86_proc *s) {
    char *buff = NULL;
    if (s) {
        buff = malloc(128);
        if (buff)
            snprintf(buff, 127, "%d", x86_proc_threads(s) );
    }
    return buff;
}

static char* x86_proc_cores_str(x86_proc *s) {
    char *buff = NULL;
    if (s) {
        buff = malloc(128);
        if (buff)
            snprintf(buff, 127, "%d", x86_proc_cores(s) );
    }
    return buff;
}

static char* x86_proc_count_str(x86_proc *s) {
    char *buff = NULL;
    if (s) {
        buff = malloc(128);
        if (buff)
            snprintf(buff, 127, "%d", x86_proc_count(s) );
    }
    return buff;
}

#define ADDFIELD(t, l, o, n, f) fields_update_bytag(s->fields, t, l, o, n, (rpiz_fields_get_func)f, (void*)s)
#define ADDFIELDSTR(t, l, o, n, str) fields_update_bytag(s->fields, t, l, o, n, NULL, (void*)str)
rpiz_fields *x86_proc_fields(x86_proc *s) {
    if (s) {
        if (!s->fields) {
            /* first insert creates */
            s->fields =
            ADDFIELD("summary.proc_desc",  0, 0, "Proccesor", x86_proc_desc );
            ADDFIELD("cpu.name",           0, 0, "Proccesor Name", x86_proc_name );
            ADDFIELD("cpu.desc",           0, 0, "Proccesor Description", x86_proc_desc );
            ADDFIELD("cpu.physical_count", 0, 1, "Count", x86_proc_count_str );
            ADDFIELD("cpu.core_count",     0, 1, "Cores", x86_proc_cores_str );
            ADDFIELD("cpu.count",          0, 1, "Threads", x86_proc_threads_str );
        }
        return s->fields;
    }
    return NULL;
}
