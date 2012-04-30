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

#include <ctype.h>
#include <stdarg.h>
#include "string_obj.h"

#ifndef va_copy
# define va_copy(d, s)		(d) = (s)
#endif


char *string_alloc(int length)
{
	if (length < 0)
		return NULL;

	return m_calloc(length + 1, sizeof(char));
}

char *string_new(const char *str)
{
	char *ret_str;

	if (!str)
		return NULL;

	ret_str = string_alloc(strlen(str));
	strcpy(ret_str, str);

	return ret_str;
}

char *string_nnew(const char *str, int length)
{
	char *ret_str;

	if (!str || length < 0)
		return NULL;

	ret_str = string_alloc(length);
	strncpy(ret_str, str, length);

	return ret_str;
}

char **string_cat(char **str, const char *add_str)
{
	char *new_str = NULL;
	int len;

	if (!str || !add_str || !*add_str)
		return str;

	len = *str ? strlen(*str) : 0;
	new_str = string_alloc(len + strlen(add_str));

	if (*str)
		strcpy(new_str, *str);
	strcpy(new_str + len, add_str);

	string_free(*str);
	*str = new_str;

	return str;
}

int string_casecmp(const char *str1, const char *str2)
{
	if (!str1 || !str2)
		return 1;

	for (; *str1 && *str2 && (tolower(*str1) == tolower(*str2)); str1++, str2++);

	return (!*str1 && !*str2) ? 0 : ((tolower(*str1) < tolower(*str2)) ? -1 : 1);
}

int string_ncasecmp(const char *str1, const char *str2, int len)
{
	int i;

	if (!str1 || !str2)
		return 1;

	for (i = 0; i < len && *str1 && *str2 && (tolower(*str1) == tolower(*str2)); str1++, str2++, i++);

	return (i >= len) ? 0 : ((tolower(*str1) < tolower(*str2)) ? -1 : 1);
}

char *string_casestr(const char *str1, const char *str2)
{
	int i, len1, len2;

	if (str1)
		len1 = strlen(str1);
	else
		return NULL;

	if (str2)
		len2 = strlen(str2);
	else
		return NULL;

	for (i = 0; i <= len1 - len2; i++)
		if (!string_ncasecmp(str1 + i, str2, len2))
			return (char*)(str1 + i);

	return NULL;
}

void string_lower(char *str)
{
	if (!str)
		return;

	for (; *str; str++)
		*str = (char) tolower(*str);
}

void string_trim(char *buf)
{
	char *from, *to;

	for (from = buf, to = buf; *from; from++) 
		if (*from != ' ' && *from != '\r' && *from != '\n' && *from != '\t')  
			*(to++) = *from; 

	*to = 0;
}

void string_move(char *to, const char *from)
{
	if (!to || !from)
		return;

	while (*from) 
		*(to++) = *(from++); 

	*to = 0;
}

void string_replace_with_char(char *buf, const char *str, const char ch)
{
	char *ptr;

	if (!buf || !str || (strlen(str) == 1 && *str == ch))
		return;

	while ((ptr = strstr(buf, str)) != NULL) {
		*ptr = ch;
		
		if (strlen(str) > 1)
			string_move(ptr + 1, ptr + strlen(str));
	}
}

char **string_printf(char **str, const char *fmt, ...)
{
	int len;
	va_list ap, ap2;

	if (!str)
		return NULL;

	string_free(*str);

	va_start(ap, fmt);
	va_copy(ap2, ap);
	if ((len = vsnprintf(NULL, 0, fmt, ap2)) < 0)
		printf(_("ERROR: printf format\n"));
	va_end(ap2);

	*str = string_alloc(len);
	if (vsnprintf(*str, len + 1, fmt, ap) < 0)
		printf(_("ERROR: vnsprintf failed\n"));

	va_end(ap);
	return str;
}

char *value2string(val_str_t *list, int val)
{
	for (; list->val != -1 && list->val != val; list++);

	return list->str;
}

int string2value(val_str_t *list, const char *str)
{
	if (!str || !*str)
		return -1;

	for (; list->val != -1 && string_casecmp(list->str, str); list++);

	return list->val;
}

