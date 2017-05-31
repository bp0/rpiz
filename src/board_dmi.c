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
#include "board_dmi.h"

struct dmi_board {
    char *board_desc;

    char *board_model;
    char *board_vendor;

    rpiz_fields *fields;
};

int dmi_board_check() {
    return dir_exists("/sys/class/dmi/id/");
}

static char *get_dmi_string(char *p) {
    char fn[256];
    char *ret = NULL, *rep = NULL;
    snprintf(fn, 256, "/sys/class/dmi/id/%s", p);
    ret = get_file_contents(fn);
    if (ret) {
        while((rep = strchr(ret, '\n'))) *rep = ' ';
    }
    return ret;
}

dmi_board *dmi_board_new() {
    int dlen = 0;
    dmi_board *s = malloc( sizeof(dmi_board) );
    if (s) {
        memset(s, 0, sizeof(*s));
        s->board_model = get_dmi_string("board_name");
        if (!s->board_model) s->board_model = strdup("(Unknown)");
        s->board_vendor = get_dmi_string("board_vendor");
        if (!s->board_vendor) s->board_vendor = strdup("(Unknown)");

        dlen = strlen(s->board_model) + strlen(s->board_vendor) + 2;
        s->board_desc = malloc(dlen);
        if (s->board_desc)
            snprintf(s->board_desc, dlen, "%s %s", s->board_vendor, s->board_model);

        s->fields = NULL;
    }
    return s;
}

void dmi_board_free(dmi_board *s) {
    if (s) {
        free(s->board_desc);
        free(s->board_model);
        free(s->board_vendor);
        if (s->fields)
            fields_free(s->fields);
        free(s);
    }
}

const char *dmi_board_desc(dmi_board *s) {
    if (s)
        return s->board_desc;
    return NULL;
}

#define ADDFIELD(t, l, o, n, f) fields_update_bytag(s->fields, t, l, o, n, (rpiz_fields_get_func)f, (void*)s)
rpiz_fields *dmi_board_fields(dmi_board *s) {
    if (s) {
        if (!s->fields) {
            /* first insert creates */
            s->fields =
            ADDFIELD("board_name",    0, 0, "Board Name", dmi_board_desc );
        }
        return s->fields;
    }
    return NULL;
}

