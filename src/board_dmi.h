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

#ifndef _DMI_H_
#define _DMI_H_

#include "fields.h"

int dmi_board_check(void);

typedef struct dmi_board dmi_board;

dmi_board *dmi_board_new(void);
void dmi_board_free(dmi_board *);

const char *dmi_board_desc(dmi_board *);
const char *dmi_board_model(dmi_board *);
const char *dmi_board_vendor(dmi_board *);

rpiz_fields *dmi_board_fields(dmi_board *);

#endif
