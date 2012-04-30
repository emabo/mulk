/*---------------------------------------------------------------------------
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 - Emanuele Bovisio
 *
 * This file is part of Mulk.
 *
 * Mulk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mulk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Mulk.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *---------------------------------------------------------------------------*/

#ifndef _STRING_OBJ_H_
#define _STRING_OBJ_H_

#include "defines.h"
#include <string.h>

#define string_free(str) \
do {if (str) {m_free(str); str = NULL;}} while(0)

typedef struct val_str_t {
	int val;
	char *str;
} val_str_t;

char *string_alloc(int length);
char *string_new(const char *str);
char *string_nnew(const char *str, int length);

char **string_cat(char **str, const char *add_str);

int string_casecmp(const char *str1, const char *str2);
int string_ncasecmp(const char *str1, const char *str2, int len);
char *string_casestr(const char *str1, const char *str2);

void string_lower(char *str);
void string_trim(char *buf);

void string_move(char *to, const char *from);

void string_replace_with_char(char *buf, const char *str, const char ch);

char **string_printf(char **str, const char *fmt, ...);

char *value2string(val_str_t *list, int val);
int string2value(val_str_t *list, const char *str);

#endif /* not _STRING_OBJ_H_ */
