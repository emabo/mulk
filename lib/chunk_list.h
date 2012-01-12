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

#ifndef _CHUNK_LIST_H_
#define _CHUNK_LIST_H_

#include <metalink/metalink_parser.h>
#include "defines.h"
#ifdef ENABLE_CHECKSUM
#include "checksum.h"
#endif

#define CHUNK_NUMBER 20L
#define MIN_CHUNK_SIZE 30000L
#define MAX_CHUNK_SIZE 3000000L


struct metalink_file_list_t;
struct metalink_resource_list_t;

typedef struct chunk_t {
	struct chunk_t *next;
	struct metalink_file_list_t *file;
	struct metalink_resource_list_t* used_res;
	off_t start;
	off_t length;
	off_t pos;
#ifdef ENABLE_CHECKSUM
	metalink_piece_hash_t *piece_hash;
	checksum_t checksum;
	int checksum_correct;
	int checksum_computed;
#endif /* ENABLE_CHECKSUM */
} chunk_t;


void reset_chunk(chunk_t *chunk);

#ifdef ENABLE_CHECKSUM
void reset_chunks_cs_none(struct metalink_file_list_t *file);

mulk_type_return_t init_chunks(struct metalink_file_list_t *metalink_file, char **newfilename);
#endif /* ENABLE_CHECKSUM */

void print_chunks(struct metalink_file_list_t *file);

chunk_t *find_free_chunk(struct metalink_file_list_t *file);

int is_chunk_downloaded(chunk_t *chunk);

int is_file_downloaded(struct metalink_file_list_t *file);

mulk_type_return_t file_statistics(struct metalink_file_list_t *file, int *chunk_completed, int *chunk_total,
	off_t *byte_downloaded, off_t *byte_total);

mulk_type_return_t create_chunks(struct metalink_file_list_t *file);

void free_chunks(struct metalink_file_list_t *file);


#endif /* not _CHUNK_LIST_H_ */
