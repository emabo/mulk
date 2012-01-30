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


#ifndef _COUNTRY_CODES_H_
#define _COUNTRY_CODES_H_

#include "defines.h"

/* country_codes.c is automatically generated starting from the following table:
 -http://en.wikipedia.org/wiki/List_of_countries_by_continent_(data_file)
*/

typedef struct continent_code_t {
	char *continent_code;
	char *continent_name;
} continent_code_t;

typedef struct country_code_t {
	char *continent_code;
	char *two_letter_code;
	char *three_letter_code;
	char *three_digit_code;
	char *country_name;
} country_code_t;

extern continent_code_t continents[];
extern country_code_t countries[];


void printf_locations(void);
void printf_continents(void);

#endif /* _COUNTRY_CODES_H_ */
