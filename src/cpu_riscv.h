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

#ifndef _RISCVCPU_H_
#define _RISCVCPU_H_

#include "fields.h"

#include "riscv_data.h"
const char *riscv_flag_list(void);

typedef struct riscv_proc riscv_proc;

riscv_proc *riscv_proc_new(void);
void riscv_proc_free(riscv_proc *);

const char *riscv_proc_name(riscv_proc *);
const char *riscv_proc_desc(riscv_proc *);
int riscv_proc_has_flag(riscv_proc *, const char *flag); /* returns core count with flag */
int riscv_proc_cores(riscv_proc *);
int riscv_proc_core_from_id(riscv_proc *, int id); /* -1 if not found */
int riscv_proc_core_id(riscv_proc *, int core);
int riscv_proc_core_khz_min(riscv_proc *, int core);
int riscv_proc_core_khz_max(riscv_proc *, int core);
int riscv_proc_core_khz_cur(riscv_proc *, int core);

rpiz_fields *riscv_proc_fields(riscv_proc *);

#endif
