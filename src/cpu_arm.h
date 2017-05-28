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

#ifndef _ARMCPU_H_
#define _ARMCPU_H_

#include "arm_data.h"
const char *arm_flag_list(void);

typedef struct arm_proc arm_proc;

arm_proc *arm_proc_new(void);
void arm_proc_free(arm_proc *);

const char *arm_proc_name(arm_proc *);
const char *arm_proc_desc(arm_proc *);
int arm_proc_has_flag(arm_proc *, const char *flag); /* returns core count with flag */
int arm_proc_cores(arm_proc *);
int arm_proc_core_from_id(arm_proc *, int id); /* -1 if not found */
int arm_proc_core_id(arm_proc *, int core);
int arm_proc_core_khz_min(arm_proc *, int core);
int arm_proc_core_khz_max(arm_proc *, int core);
int arm_proc_core_khz_cur(arm_proc *, int core);

#endif
