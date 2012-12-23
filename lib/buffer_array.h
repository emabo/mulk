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

#ifndef _BUFFER_ARRAY_H_
#define _BUFFER_ARRAY_H_

#include "defines.h"
#ifdef ENABLE_METALINK
#include "metalink_list.h"
#endif
#include "url_list.h"

/* file struct */
typedef struct buffer_t {
	CURL *id;
	FILE *file_pt;
	url_list_t *url;
	UriUriA *uri;
	char *filename;

#ifdef ENABLE_METALINK
	metalink_resource_list_t* used_res;
	chunk_t *chunk;
#endif
} buffer_t;

/* url struct */
extern buffer_t *buffer_array;

void create_buffer_array(void);

#ifdef ENABLE_METALINK
int count_number_of_chunks(metalink_file_list_t *file);

int open_buffer(CURL *id, url_list_t *url, UriUriA *uri, chunk_t *chunk, metalink_resource_list_t *resource,
	int header);
#else
int open_buffer(CURL *id, url_list_t *url, UriUriA *uri);
#endif

int get_buffer(CURL *id);

int count_number_of_hostname(UriUriA *uri);

int is_uri_compatible(UriUriA *uri, int maxconns);

mulk_type_return_t close_buffer(CURL *id, CURLcode err_code, long resp_code, int *file_completed);

void print_buffers(void);

void free_buffer_array(CURLM *curl_obj);

void set_buffer_mime_type(CURL *id, const char *mimetype);

int is_uri_http_buffer(CURL *id);

#endif /* not _BUFFER_ARRAY_H_ */
