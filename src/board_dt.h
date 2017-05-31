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

#ifndef _DT_H_
#define _DT_H_

#include "fields.h"

int dt_board_check(void);
char *get_dt_string(char *p);

typedef struct dt_board dt_board;

dt_board *dt_board_new(void);
void dt_board_free(dt_board *);

const char *dt_board_desc(dt_board *);
const char *dt_board_serial(dt_board *);

rpiz_fields *dt_board_fields(dt_board *);

#endif
