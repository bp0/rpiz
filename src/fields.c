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
#include "fields.h"

struct rpiz_fields {
    int live;
    int own_value;
    char *tag;
    char *name;
    char *value;
    void *data;
    rpiz_fields_get_func get_func;
    rpiz_fields *next;
};

rpiz_fields *fields_new() {
    rpiz_fields *s = malloc(sizeof(rpiz_fields));
    if (s) {
        memset(s, 0, sizeof(*s));
    }
    return s;
}

rpiz_fields *fields_next(rpiz_fields *s) {
    if (s)
        return s->next;
    else
        return NULL;
}

static void fields_update(rpiz_fields *s, int live_update, int own_value, char *name, rpiz_fields_get_func get_func, void *data) {
    if (s) {
        if (name) {
            free(s->name);
            s->name = strdup(name);
        }
        s->live = live_update;
        s->own_value = own_value;
        s->get_func = get_func;
        s->data = data;
        fields_get(s, NULL, NULL, NULL);
    }
}


/* returns NULL or new item */
rpiz_fields *fields_update_bytag(rpiz_fields *s, char *tag, int live_update, int own_value, char *name, rpiz_fields_get_func get_func, void *data) {
    int m = 0;
    rpiz_fields *nf = NULL;
    if (tag == NULL) return NULL;
    if (s) {
        if (tag && s->tag)
            m = !!(strcmp(tag, s->tag) == 0);
        if (m) {
            /* update existing */
            fields_update(s, live_update, own_value, name, get_func, data);
            return NULL;
        } else {
            nf = fields_update_bytag(s->next, tag, live_update, own_value, name, get_func, data);
            if (nf) s->next = nf;
            else return NULL;
        }
    } else {
        /* new */
        s = fields_new();
        if (s) {
            s->tag = strdup(tag);
            fields_update(s, live_update, own_value, name, get_func, data);
            return s;
        }
    }
    return NULL;
}

int fields_islive(rpiz_fields *s, char *tag) {
    int m = 0;
    if (s) {
        if (tag && s->tag)
            m = !!(strcmp(tag, s->tag) == 0);
        if (m)
            return s->live;
        if (s->next)
            return fields_islive(s->next, tag);
    }
    return 0;
}

int fields_get(rpiz_fields *s, char **tag, char **name, char **value) {
    char *tmp;
    if (s) {
        if (tag) *tag = s->tag;
        if (name) *name = s->name;
        if (s->get_func) {
            tmp = s->get_func(s->data);
            if (s->value)
                free(s->value);
            if (tmp) {
                if (s->own_value)
                    s->value = tmp;
                else {
                    s->value = strdup(tmp);
                }
            } else
                s->value = NULL;
        }
        if (value) *value = s->value;
        return 1;
    }
    return 0;
}

int fields_get_bytag(rpiz_fields *s, char *tag, char **name, char **value) {
    int m = 0;
    if (s) {
        if (tag && s->tag)
            m = !!(strcmp(tag, s->tag) == 0);
        if (m)
            return fields_get(s, NULL, name, value);
        if (s->next)
            return fields_get_bytag(s->next, tag, name, value);
    }
    return 0;
}

void fields_free(rpiz_fields *s) {
    if (s) {
        if (s->next != NULL)
            fields_free(s->next);
        free(s->tag);
        free(s->name);
        free(s->value);
        free(s);
    }
}
