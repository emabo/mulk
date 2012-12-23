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

#ifndef _URL_LIST_H_
#define _URL_LIST_H_

#include "defines.h"
#ifdef ENABLE_METALINK
#include "metalink_list.h"
#endif

#define ERR_CODE_NOT_ASSIGNED -1
#define ERR_CODE_EMPTY_URL     10000


typedef struct url_list_t {
	int id;
	int level;
	int tmp_file_created;
	UriUriA *uri;
	char *mimetype;
	struct url_list_t *next;
	
	long err_code;
	long http_code;
	char *filename;
	char *mimefilename;

	int assigned;
	int reported;

#ifdef ENABLE_METALINK
	/* metalink */
	metalink_file_list_t *metalink_uri;
#endif
} url_list_t;


int is_url_list_empty(void);

UriUriA *create_absolute_url(const char *base_url, const char *url);

#ifdef ENABLE_RECURSION
int add_new_url_and_check(const url_list_t *base_elem, const char *url, char **relative_url);
#endif /* ENABLE_RECURSION */

#ifdef ENABLE_METALINK
url_list_t *search_next_url(UriUriA **uri, chunk_t **chunk, metalink_resource_list_t **resource,
	int *header);

void push_metalink_uri(metalink_file_list_t *metalink_uri, int level);

void set_url_file_length(url_list_t *url, off_t size);
#else /* not ENABLE_METALINK */
url_list_t *search_next_url(UriUriA **uri);
#endif /* not ENABLE_METALINK */

/* remember to free the url */
UriUriA *pop_url(void);

void report_urls(void);

void reset_url(url_list_t *url);
void reset_url_list(void);

void free_urls(void);

#endif /* not _URL_LIST_H_ */
