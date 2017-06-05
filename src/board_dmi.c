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
    char *board_version;
    char *board_serial;

    char *bios_vendor;
    char *bios_version;
    char *bios_date;

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
        while((rep = strchr(ret, '\n'))) *rep = 0;
        //DEBUG printf("get_dmi_string( %s ): (len:%d) %s\n", p, (int)strlen(ret), ret);
    }
    return ret;
}

#define DMI_GET(v,s) v = get_dmi_string(s);
#define DMI_GET_UNK(v,s) \
        v = get_dmi_string(s); \
        if (!v) v = strdup("(Unknown)");

dmi_board *dmi_board_new() {
    int dlen = 0;
    dmi_board *s = malloc( sizeof(dmi_board) );
    if (s) {
        memset(s, 0, sizeof(*s));
        DMI_GET_UNK(s->board_model,   "board_name");
        DMI_GET_UNK(s->board_vendor,  "board_vendor");
        DMI_GET_UNK(s->board_version, "board_version");
        DMI_GET(s->board_serial, "board_serial");
        DMI_GET_UNK(s->bios_date,   "bios_date");
        DMI_GET_UNK(s->bios_vendor,  "bios_vendor");
        DMI_GET_UNK(s->bios_version, "bios_version");

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
        free(s->board_version);
        free(s->board_serial);
        free(s->bios_date);
        free(s->bios_version);
        free(s->bios_vendor);
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

const char *dmi_board_model(dmi_board *s) {
    if (s)
        return s->board_model;
    return NULL;
}

const char *dmi_board_vendor(dmi_board *s) {
    if (s)
        return s->board_vendor;
    return NULL;
}

const char *dmi_board_version(dmi_board *s) {
    if (s)
        return s->board_version;
    return NULL;
}

const char *dmi_board_serial(dmi_board *s) {
    if (s)
        return s->board_serial;
    return NULL;
}

const char *dmi_board_bios_vendor(dmi_board *s) {
    if (s)
        return s->bios_vendor;
    return NULL;
}

const char *dmi_board_bios_version(dmi_board *s) {
    if (s)
        return s->bios_version;
    return NULL;
}

const char *dmi_board_bios_date(dmi_board *s) {
    if (s)
        return s->bios_date;
    return NULL;
}

#define ADDFIELD(t, l, o, n, f) fields_update_bytag(s->fields, t, l, o, n, (rpiz_fields_get_func)f, (void*)s)
rpiz_fields *dmi_board_fields(dmi_board *s) {
    if (s) {
        if (!s->fields) {
            /* first insert creates */
            s->fields =
            ADDFIELD("summary.board_name", 0, 0, "Board Name", dmi_board_desc );
            ADDFIELD("board.dmi_model",    0, 0, "Board Model", dmi_board_model );
            ADDFIELD("board.dmi_vendor",   0, 0, "Board Vendor", dmi_board_vendor );
            ADDFIELD("board.dmi_version",  0, 0, "Board Version", dmi_board_version );
            ADDFIELD("board.dmi_serial",   0, 0, "Board Serial Number", dmi_board_serial );
            ADDFIELD("board.dmi_bios_vendor",  0, 0, "BIOS Vendor", dmi_board_bios_vendor );
            ADDFIELD("board.dmi_bios_version", 0, 0, "BIOS Version", dmi_board_bios_version );
            ADDFIELD("board.dmi_bios_date",    0, 0, "BIOS Date", dmi_board_bios_date );
        }
        return s->fields;
    }
    return NULL;
}

