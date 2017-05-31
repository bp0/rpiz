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
#include "board_dt.h"

struct dt_board {
    char *board_model;
    char *board_serial;

    rpiz_fields *fields;
};

char *get_dt_string(char *p) {
    char fn[256];
    char *ret = NULL, *rep = NULL;
    snprintf(fn, 256, "/proc/device-tree/%s", p);
    ret = get_file_contents(fn);
    if (ret) {
        while((rep = strchr(ret, '\n'))) *rep = ' ';
    }
    return ret;
}

int dt_board_check() {
    char *dtm;
    int ret = 0;
    dtm = get_dt_string("model");
    if (dtm) {
        ret = 1;
        free(dtm);
    }
    return ret;
}

dt_board *dt_board_new() {
    dt_board *s = malloc( sizeof(dt_board) );
    if (s) {
        memset(s, 0, sizeof(*s));
        s->board_model = get_dt_string("model");
        if (!s->board_model) s->board_model = strdup("(Unknown)");
        s->board_serial = get_dt_string("serial-number");
        if (!s->board_serial) s->board_serial = strdup("");

        s->fields = NULL;
    }
    return s;
}

void dt_board_free(dt_board *s) {
    if (s) {
        free(s->board_model);
        free(s->board_serial);
        if (s->fields)
            fields_free(s->fields);
        free(s);
    }
}

const char *dt_board_desc(dt_board *s) {
    if (s)
        return s->board_model;
    return NULL;
}

const char *dt_board_serial(dt_board *s) {
    if (s)
        return s->board_serial;
    return NULL;
}

#define ADDFIELD(t, l, o, n, f) fields_update_bytag(s->fields, t, l, o, n, (rpiz_fields_get_func)f, (void*)s)
rpiz_fields *dt_board_fields(dt_board *s) {
    if (s) {
        if (!s->fields) {
            /* first insert creates */
            s->fields =
            ADDFIELD("board_name",    0, 0, "Board Name", dt_board_desc );
            ADDFIELD("board_serial",  0, 0, "Serial Number", dt_board_serial );
        }
        return s->fields;
    }
    return NULL;
}

