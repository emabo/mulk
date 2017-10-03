/*---------------------------------------------------------------------------
 * Copyright (C) 2008-2017 - Emanuele Bovisio
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

#ifndef _URI_PARSER_H_
#define _URI_PARSER_H_

#include "defines.h"

#define DOMAIN_OPTION_DELIM ", "
#define DOMAIN_SEPARATOR '.'


char *uri2string(UriUriA *uri);
char *uri2filename(UriUriA *uri);
void uri_free(UriUriA *uri);
UriUriA *create_absolute_uri(const UriUriA *base_uri, const char *url);
int	filter_uri(UriUriA **uri, int level);

char *get_host(UriUriA *uri);

int is_uri_protocol(UriUriA *uri, const char *protocol);
int is_uri_http(UriUriA *uri);
int is_uri_ftp(UriUriA *uri);

int are_hosts_equal(UriUriA *first, UriUriA *second);

int is_host_equal_domain(const char *host, const char *domain);
int is_host_in_domain(const char *host, const char *domain);
int is_host_equal_domains(const char *host, char **domains);
int is_host_in_domains(const char *host, char **domains);

UriUriA *filename2absolute_uri(const char *abs_filename);
char *uri2absolute_filename(UriUriA* abs_uri);
char *extract_relative_url(const char *src_filename, const char *base_filename);

#endif /* not _URI_PARSER_H_ */
