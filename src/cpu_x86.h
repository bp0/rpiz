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

#ifndef _X86CPU_H_
#define _X86CPU_H_

#include "fields.h"

//#include "arm_data.h"
//const char *arm_flag_list(void);

typedef struct x86_proc x86_proc;

x86_proc *x86_proc_new(void);
void x86_proc_free(x86_proc *);

const char *x86_proc_name(x86_proc *);
const char *x86_proc_desc(x86_proc *);
int x86_proc_has_flag(x86_proc *, const char *flag); /* returns core count with flag */
int x86_proc_cores(x86_proc *);
int x86_proc_core_from_id(x86_proc *, int id); /* -1 if not found */
int x86_proc_core_id(x86_proc *, int core);
int x86_proc_core_khz_min(x86_proc *, int core);
int x86_proc_core_khz_max(x86_proc *, int core);
int x86_proc_core_khz_cur(x86_proc *, int core);

rpiz_fields *x86_proc_fields(x86_proc *);

#endif
