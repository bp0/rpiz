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
#include <dirent.h>
#include "util.h"

char *get_file_contents(const char *file) {
    FILE *fh;
    char *buff = NULL;
    char *loc = NULL;
    int rlen = 0;
    int pages = 1;
    int fs = 0;

    fh = fopen(file, "r");
    if (!fh)
        return NULL;

    buff = malloc(pages * 1024 + 1);
    if (buff == NULL) {
        fclose(fh);
        return NULL;
    }

    loc = buff;
    while((rlen = fread(loc, 1, 1024, fh))) {
        fs += rlen;
        if (rlen == 1024) {
            pages++;
            buff = realloc(buff, pages * 1024 + 1);
            if (buff == NULL) {
                fclose(fh);
                return NULL;
            }
            loc = buff + ((pages-1) * 1024);
        } else
            break;
    }
    fclose(fh);
    buff[fs+1] = '\0';

    //DEBUG printf("get_file_contents( %s ): fs: %d, pages: %d\n", file, fs, pages);

    return buff;
}

int dir_exists(const char* path) {
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else
        return 0;
}

int get_cpu_int(const char* item, int cpuid) {
    char fn[256];
    char *fc = NULL;
    int ret = 0;
    snprintf(fn, 256, "/sys/devices/system/cpu/cpu%d/%s", cpuid, item);
    fc = get_file_contents(fn);
    if (fc) {
        ret = atol(fc);
        free(fc);
    }
    return ret;
}

int get_cpu_freq(int id, int *min, int *max, int *cur) {
    int ret = 0;
    if (min)
        ret += *min = get_cpu_int("cpufreq/scaling_min_freq", id);
    if (max)
        ret += *max = get_cpu_int("cpufreq/scaling_max_freq", id);
    if (cur)
        ret += *cur = get_cpu_int("cpufreq/scaling_cur_freq", id);
    return !!ret;
}

cpu_string_list *strlist_new(void) {
    cpu_string_list *list = malloc( sizeof(cpu_string_list) );
    list->count = 0;
    list->strs = NULL;
    return list;
}

void strlist_free(cpu_string_list *list) {
    int i;
    for (i = 0; i < list->count; i++) {
        free(list->strs[i].str);
    }
    free(list);
    list = NULL;
}

char *strlist_add_w(cpu_string_list *list, const char* str, int weight) {
    int i;
    for (i = 0; i < list->count; i++) {
        if (strcmp(list->strs[i].str, str) == 0) {
            /* found */
            list->strs[i].ref_count += weight;
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
    list->strs[i].ref_count = weight;
    return list->strs[i].str;
}

char *strlist_add(cpu_string_list *list, const char* str) {
    return strlist_add_w(list, str, 1);
}

#define MAXLEN_KEY 128
#define MAXLEN_VALUE 512

struct kv_scan {
    char *buffer;
    int own_buffer;
    char *curline, *nextline;
    char key[MAXLEN_KEY], value[MAXLEN_VALUE];
};

kv_scan *kv_new(char *buffer) {
    kv_scan *s = NULL;
    if (buffer) {
        s = malloc( sizeof(kv_scan) );
        if (s) {
            memset(s, 0, sizeof(*s));
            s->buffer = buffer;
            s->own_buffer = 0;
            s->curline = s->buffer;
            s->nextline = strchr(s->curline, '\n');
        }
    }
    return s;
}

kv_scan *kv_new_file(const char *file) {
    kv_scan *s = NULL;
    s = malloc( sizeof(kv_scan) );
    if (s) {
        memset(s, 0, sizeof(*s));
        s->buffer = get_file_contents(file);
        if (s->buffer) {
            s->own_buffer = 1;
            s->curline = s->buffer;
            s->nextline = strchr(s->curline, '\n');
        } else {
            free(s);
            return NULL;
        }
    }
    return s;
}

int kv_next(kv_scan *s, char **k, char **v) {
    int klen, vlen, found = 0;
    char *nextcol = NULL;
    if (s) {
        *k = NULL; *v = NULL;
        while(s->nextline != NULL) {
            nextcol = strchr(s->curline, ':');
            if (nextcol != NULL && nextcol < s->nextline) {
                klen = nextcol - s->curline;
                nextcol++; while (*nextcol == ' ') nextcol++; /* skip : and any leading spaces */
                vlen = s->nextline - nextcol;
                if (klen > MAXLEN_KEY-1) klen = MAXLEN_KEY-1;
                if (vlen > MAXLEN_VALUE-1) vlen = MAXLEN_VALUE-1;
                memset(s->key, 0, MAXLEN_KEY);
                memset(s->value, 0, MAXLEN_VALUE);
                strncpy(s->key, s->curline, klen);
                strncpy(s->value, nextcol, vlen);
                *k = s->key; *v = s->value; found = 1;
            }
            s->curline = s->nextline + 1;
            s->nextline = strchr(s->curline, '\n');
            if (found)
                return 1;
        }
        return 0;
    } else
        return 0;
}

void kv_free(kv_scan *s) {
    if (s) {
        if (s->own_buffer)
            free(s->buffer);
        free(s);
    }
}
