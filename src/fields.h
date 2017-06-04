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

#ifndef _FIELDS_H_
#define _FIELDS_H_

typedef struct rpiz_fields rpiz_fields;

typedef char* (*rpiz_fields_get_func)(void *data);

rpiz_fields *fields_new(void);
rpiz_fields *fields_copy(rpiz_fields *src, rpiz_fields *append_src);
rpiz_fields *fields_next(rpiz_fields *);

int fields_tag_has_prefix(rpiz_fields *, const char *prefix);
rpiz_fields *fields_next_with_tag_prefix(rpiz_fields *, const char *prefix);

rpiz_fields *fields_update_bytag(rpiz_fields *, char *tag, int live_update, int own_value, char *name, rpiz_fields_get_func get_func, void *data);
int fields_islive(rpiz_fields *, char *tag);
int fields_get(rpiz_fields *, char **tag, char **name, char **value);
int fields_get_bytag(rpiz_fields *, char *tag, char **name, char **value);
void fields_free(rpiz_fields *);

void fields_dump(rpiz_fields *);

#endif
