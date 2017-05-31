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
#include "board.h"
#include "board_rpi.h"
#include "board_dmi.h"

typedef enum {
    BT_UNKNOWN = 0,
    BT_RPI,
    BT_DMI,
    BT_N_TYPES,
} board_type;

struct {
    board_type type;
    rpi_board *rpi;
    dmi_board *dmi;
} board;

int board_init() {
    if (rpi_board_check()) {
        board.rpi = rpi_board_new();
        board.type = BT_RPI;
    } else if (dmi_board_check()) {
        board.dmi = dmi_board_new();
        board.type = BT_DMI;
    } else
        board.type = BT_UNKNOWN;
    return 1;
}

void board_cleanup() {
    if (board.rpi) rpi_board_free(board.rpi);
    if (board.dmi) dmi_board_free(board.dmi);
}

rpiz_fields *board_fields() {
    switch (board.type) {
        case (BT_RPI):
            return rpi_board_fields(board.rpi);
        case (BT_DMI):
            return dmi_board_fields(board.dmi);
        default:
            return NULL;
    }
    return NULL;
}
