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

#ifndef _METALINK_LIST_H_
#define _METALINK_LIST_H_

#include "defines.h"
#include <metalink/metalink_parser.h>
#include "chunk_list.h"
#ifdef ENABLE_CHECKSUM
#include "checksum.h"
#endif


typedef enum metalink_res_code_t {
	METALINK_RES_OK =                1000,
	METALINK_RES_NO_NORE_RESOURCES = 1001,
	METALINK_RES_WRONG_CHECKSUM =    1002,
	METALINK_RES_INVALID_METALINK =  1003,
} metalink_res_code_t;

typedef struct metalink_resource_list_t {
	struct metalink_resource_list_t *prev, *next;
	metalink_resource_t* resource;
	UriUriA *uri;
	int assigned;
	int error;
} metalink_resource_list_t;

typedef struct metalink_file_list_t {
	metalink_file_t *file;
	off_t size;
	int header;
	metalink_resource_list_t *usable_res_top;
	metalink_resource_list_t *usable_res_bottom;
	int chunk_number;
	chunk_t *chunk;
	char *resume_filename;
} metalink_file_list_t;

typedef struct metalink_list_t {
	struct metalink_list_t *next;
	metalink_t* metalink;
} metalink_list_t;


void remove_metalink_resource(metalink_file_list_t *file, metalink_resource_list_t *resource);

int is_resource_available(metalink_file_list_t *file, int header);

void free_resources(metalink_file_list_t *file);

metalink_file_list_t *create_metalink_file(metalink_file_t *file);

UriUriA *find_next_url(metalink_file_list_t *file, chunk_t **chunk, metalink_resource_list_t **resource, 
	int *header);

void set_metalink_file_length(metalink_file_list_t *file, off_t size);

void free_metalink_file(metalink_file_list_t *file);

void reset_metalink_file(metalink_file_list_t *file, const char *resume_filename);

int is_valid_metalink(metalink_file_t* file);

void push_metalink(metalink_t *metalink);

void free_metalinks(void);

mulk_type_return_t add_new_metalink(const char *filename, int level);

#endif /* not _METALINK_LIST_H_ */
