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

#ifndef _RPI_H_
#define _RPI_H_

typedef struct rpi_board rpi_board;

rpi_board *rpi_board_new(void);
void rpi_board_free(rpi_board *);

const char *rpi_board_desc(rpi_board *);
const char *rpi_board_rcode(rpi_board *);
const char *rpi_board_serial(rpi_board *);
const char *rpi_board_model(rpi_board *);
const char *rpi_board_rev(rpi_board *);
const char *rpi_board_intro(rpi_board *);
const char *rpi_board_mfgby(rpi_board *);
const char *rpi_board_mem_spec(rpi_board *);
const char *rpi_board_soc(rpi_board *);
int rpi_board_overvolt(rpi_board *);
float rpi_soc_temp(void);

#endif
