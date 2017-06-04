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

void fields_free(rpiz_fields *s) {
    if (s) {
        if (s->next != NULL)
            fields_free(s->next);
        free(s->tag);
        free(s->name);
        if (s->own_value)
            free(s->value);
        free(s);
    }
}

rpiz_fields *fields_copy(rpiz_fields *src, rpiz_fields *append_src) {
    rpiz_fields *dest = NULL, *prev = NULL, *cpd = NULL;
    while (src) {
        cpd = malloc(sizeof(rpiz_fields));
        if (!cpd) {
            fields_free(dest);
            return NULL;
        }
        memset(cpd, 0, sizeof(*cpd));
        if (!dest)
            dest = cpd;
        if (prev)
            prev->next = cpd;

        cpd->own_value = src->own_value;
        cpd->live = src->live;
        cpd->get_func = src->get_func;
        cpd->data = src->data;
        cpd->tag = strdup(src->tag);
        cpd->name = strdup(src->name);
        if (cpd->own_value)
            cpd->value = strdup(src->value);
        else
            cpd->value = src->value;

        prev = cpd;
        src = src->next;
        if (!src && append_src) {
            /* append another fields list if given */
            src = append_src;
            append_src = NULL;
        }
    }
    return dest;
}


int fields_tag_has_prefix(rpiz_fields *s, const char *prefix) {
    int tl = 0, pl = 0;
    if (s && prefix) {
        pl = strlen(prefix);
        if (s->tag)
            tl = strlen(s->tag);
        if (tl && tl >= pl)
            if (strncmp(s->tag, prefix, pl) == 0)
                return 1;
    }
    return 0;
}

rpiz_fields *fields_next(rpiz_fields *s) {
    if (s)
        return s->next;
    else
        return NULL;
}

rpiz_fields *fields_next_with_tag_prefix(rpiz_fields *s, const char *prefix) {
    if (s)
        s = s->next;
    while(s) {
        if ( fields_tag_has_prefix(s, prefix) )
            return s;
        s = s->next;
    }
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
            if (s->value && s->own_value)
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

void fields_dump(rpiz_fields *s) {
    char *t, *n, *v;
    rpiz_fields *f = s;
    while (f) {
        fields_get(f, &t, &n, &v);
        printf("[%s] %s = %s\n", t, n, v);
        f = fields_next(f);
    }
}
