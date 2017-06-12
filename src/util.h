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

#ifndef _UTIL_H_
#define _UTIL_H_

char *get_file_contents(const char *file);
int dir_exists(const char* path);

/* -- /sys/devices/system/cpu/.. -- */
int get_cpu_int(const char* item, int cpuid);
char *get_cpu_str(const char* item, int cpuid);
int get_cpu_freq(int id, int *min, int *max, int *cur);

/* -- string structures used in cpu_*  -- */

typedef struct {
    int ref_count;
    char *str;
} cpu_string;

typedef struct {
    int count;
    cpu_string *strs;
} cpu_string_list;

cpu_string_list *strlist_new(void);
void strlist_free(cpu_string_list *list);
char *strlist_add_w(cpu_string_list *list, const char* str, int weight);
char *strlist_add(cpu_string_list *list, const char* str);

/* -- key / value scan  -- */
typedef struct kv_scan kv_scan;

kv_scan *kv_new(char *buffer);
kv_scan *kv_new_file(const char *file);
int kv_next(kv_scan *, char **key, char **value);
void kv_free(kv_scan *);

#endif
