/*---------------------------------------------------------------------------
 * Copyright (C) 2008, 2009, 2010, 2011 - Emanuele Bovisio
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

#include "metalink_list.h"
#include "chunk_list.h"
#include "string_obj.h"
#include "url_list.h"
#include "buffer_array.h"
#include "file_obj.h"

void reset_chunk(chunk_t *chunk)
{
	if (!chunk)
		return;

	chunk->pos = 0;

#ifdef ENABLE_CHECKSUM
	if (chunk->checksum.cs_type != CS_NONE)
		init_context(&chunk->checksum);

	chunk->checksum_correct = 0;
	chunk->checksum_computed = 0;
#endif
}

int is_chunk_downloaded(chunk_t *chunk)
{
	int ret = 1;

	if (!chunk)
		return 0;

	if (chunk->pos != chunk->length)
		return 0;

#ifdef ENABLE_CHECKSUM
	if (chunk->checksum.cs_type != CS_NONE) {
		char *str_checksum;

		if (chunk->checksum_computed)
			return chunk->checksum_correct;

		final_context(&chunk->checksum);

		str_checksum = str_digest(&chunk->checksum);

		chunk->checksum_correct = str_checksum && !strcmp(str_checksum, chunk->piece_hash->hash);
		chunk->checksum_computed = 1;

		if (chunk->checksum_correct) 
			MULK_NOTE((_("Chunk checksum correct.\n")));
		else 
			MULK_NOTE((_("Chunk checksum wrong.\n")));

		ret = chunk->checksum_correct;
	}
#endif /* ENABLE_CHECKSUM */

	return ret;
}

#ifdef ENABLE_CHECKSUM
static void push_chunk(metalink_file_list_t *file, long start, long length, 
	checksum_type_t cs, metalink_piece_hash_t *piece_hash)
#else
static void push_chunk(metalink_file_list_t *file, long start, long length)
#endif
{
	chunk_t *elem;

	if (!file)
		return;

	elem = m_calloc(1, sizeof(chunk_t));
	elem->file = file;
	elem->start = start;
	elem->length = length;
	elem->next = file->chunk;
#ifdef ENABLE_CHECKSUM
	elem->checksum.cs_type = cs;
	elem->piece_hash = piece_hash;
#endif /* ENABLE_CHECKSUM */

	reset_chunk(elem);

	file->chunk = elem;
}

void print_chunks(metalink_file_list_t *file)
{
	chunk_t *elem;
	int i = 0;

	if (!file || !is_printf(MINFO))
		return;

	for (elem = file->chunk; elem; elem = elem->next) {
		char *uri_str = elem->used_res ? uri2string(elem->used_res->uri) : "NULL";

		printf("%d: %ld %ld-%ld/%ld, %s\n", ++i, (long)elem->pos, (long)elem->start,
			(long)elem->start+elem->length-1, (long)elem->length, uri_str);

		if (elem->used_res)
			string_free(&uri_str);
	} 
}

chunk_t *find_free_chunk(metalink_file_list_t *file)
{
	chunk_t *elem;

	if (!file)
		return NULL;

	for (elem = file->chunk; elem; elem = elem->next) 
		if (!elem->used_res && elem->length > elem->pos)
			return elem;

	return NULL;
}

int is_file_downloaded(metalink_file_list_t *file)
{
	chunk_t *elem;

	if (!file)
		return 0;

	if ((elem = file->chunk) == NULL)
		return 0;

	for (; elem; elem = elem->next)
		if (!is_chunk_downloaded(elem))
			return 0;

	return 1;
}

mulk_type_return_t file_statistics(metalink_file_list_t *file, int *chunk_completed, int *chunk_total,
	long *byte_downloaded, long *byte_total)
{
	chunk_t *elem;

	*chunk_completed = *chunk_total = 0;
	*byte_downloaded = *byte_total = 0;

	if (!file)
		return MULK_RET_FILE_ERR;

	*chunk_total = file->chunk_number;
	*byte_total = file->size;

	if ((elem = file->chunk) == NULL)
		return MULK_RET_ERR;

	for (; elem; elem = elem->next)
		if (is_chunk_downloaded(elem)) {
			(*chunk_completed)++;
			*byte_downloaded += elem->pos;
		}

	return MULK_RET_OK;
}

#ifdef ENABLE_CHECKSUM
static int get_number_of_piece_hashes(metalink_piece_hash_t **piece_hashes)
{
	int count = 0;

	if (!piece_hashes)
		return 0;

	while (*piece_hashes) {
		piece_hashes++;
		count++;
	}

	return count;
}

static metalink_piece_hash_t *get_piece_hash(metalink_piece_hash_t **piece_hashes, int pos)
{
	metalink_piece_hash_t **ptr;

	if (!piece_hashes)
		return NULL;

	ptr = piece_hashes + pos;
	if (*ptr && (*ptr)->piece == pos)
		return *ptr;

	/* the pieces are not ordered, we have to find the right one */
	for (ptr = piece_hashes; *ptr; ptr++)
		if ((*ptr)->piece == pos)
			return *ptr;

	return NULL;
}

static checksum_verify_type_t verify_chunk_checksum_metalink_file(metalink_file_list_t *metalink_file, const char *filename, int *num_chunk)
{
	chunk_t *chunk;
	FILE *file;
	checksum_verify_type_t verified = CS_VERIFY_OK;
	int count = 1;

	*num_chunk = 0;

	if (!metalink_file || !filename)
		return CS_VERIFY_ERR;

	if (!(file = fopen(filename, "r")))
		return CS_VERIFY_ERR;

	for (chunk = metalink_file->chunk; chunk; chunk = chunk->next) {
		MULK_NOTE((_("Checking chunk no. %d\n"), count++));

		reset_chunk(chunk);

		chunk->pos = update_context_chunk_file(file, &chunk->checksum, chunk->start, chunk->length);
		
		if (is_chunk_downloaded(chunk))
			(*num_chunk)++;
		else {
			reset_chunk(chunk);
			verified = CS_VERIFY_ERR;
		}
	}

	fclose(file);

	return verified;
}

static mulk_type_return_t init_chunks(metalink_file_list_t *metalink_file)
{
	int num_chunk;

	if (!metalink_file || !metalink_file->file->name || !metalink_file->resume_filename 
		|| !is_file_exist(metalink_file->resume_filename))
		return MULK_RET_FILE_ERR;

	if (create_truncated_file(metalink_file->resume_filename, metalink_file->size))
		return MULK_RET_FILE_ERR;

	if (verify_chunk_checksum_metalink_file(metalink_file, metalink_file->resume_filename, &num_chunk) == CS_VERIFY_OK) {
		MULK_NOTE((_("Resuming file: %s, verified %d/%d chunks.\n"), metalink_file->file->name,
			num_chunk, metalink_file->chunk_number));

		MULK_INFO((_("Saving file: %s\n"), metalink_file->file->name));

		if (verify_metalink_file(metalink_file->file, metalink_file->resume_filename) == CS_VERIFY_OK) {
			if (!make_dir_pathname(metalink_file->file->name))
				rename(metalink_file->resume_filename, metalink_file->file->name);
		} else {
			MULK_NOTE((_("The file will be deleted.\n")));
			remove(metalink_file->resume_filename);
		}
	}
	else 
		MULK_NOTE((_("Resuming file: %s, verified %d/%d chunks.\n"), metalink_file->file->name,
			num_chunk, metalink_file->chunk_number));

	return MULK_RET_OK;
}
#endif /* ENABLE_CHECKSUM */

void create_chunks(metalink_file_list_t *file)
{
	int chunk_number = CHUNK_NUMBER, i;
	long last_chunk_size = 0, chunk_size = 0, chunk_remainder, filesize;
#ifdef ENABLE_CHECKSUM
	checksum_type_t cs = CS_NONE;
#else
	int cs = 0;
#endif
	
	if (!file || file->size < 0 || !file->file)
		return;

	filesize = file->size;

#ifdef ENABLE_CHECKSUM
	/* chunk size derived from checksum piece size */
	if (file->file->chunk_checksum) {
		chunk_number = get_number_of_piece_hashes(file->file->chunk_checksum->piece_hashes);
		chunk_size = file->file->chunk_checksum->length;
		last_chunk_size = filesize - chunk_size * (chunk_number - 1);

		if (chunk_number && chunk_size > 0 && last_chunk_size > 0 && last_chunk_size < 2 * chunk_size) {
			cs = string2checksum_type(file->file->chunk_checksum->type);
			if (cs)
				MULK_NOTE((_("Found chunk checksum of %s type.\n"), checksum_type2string(cs)));
			else
				MULK_NOTE((_("No compatible checksum to verify.\n")));
		}
		else {
			cs = CS_NONE;
			MULK_ERROR((_("ERROR: wrong length in chunk checksum.\n")));
		}
	}
	else
		cs = CS_NONE;
#endif /* ENABLE_CHECKSUM */

	if (!cs) {
		chunk_size = filesize / chunk_number;
		if (chunk_size < MIN_CHUNK_SIZE) {
			chunk_number = filesize / MIN_CHUNK_SIZE;
			if (!chunk_number)
				chunk_number = 1;
			chunk_size = filesize / chunk_number;
		}
		else if (chunk_size > MAX_CHUNK_SIZE) {
			chunk_number = filesize / MAX_CHUNK_SIZE + 1;
			if (!chunk_number)
				chunk_number = 1;
			chunk_size = filesize / chunk_number;
		}

		chunk_remainder = filesize % chunk_number;
		last_chunk_size = chunk_size + chunk_remainder;
	}

	MULK_DEBUG((_("filesize: %ld\n"), filesize));
	MULK_DEBUG((_("number of chunks: %d\n"), chunk_number));
	MULK_DEBUG((_("chunk size: %ld\n"), chunk_size));
	MULK_DEBUG((_("last chunk size: %ld\n"), last_chunk_size));

	for (i = 0; i < chunk_number && filesize > 0; i++) {
		long size = i ? chunk_size : last_chunk_size;

		filesize -= size;

#ifdef ENABLE_CHECKSUM
		push_chunk(file, filesize >= 0 ? filesize : 0, size, cs, 
			cs ? get_piece_hash(file->file->chunk_checksum->piece_hashes, chunk_number - i - 1) : NULL);
#else
		push_chunk(file, filesize >= 0 ? filesize : 0, size);
#endif
	}
	file->chunk_number = chunk_number;

	/* load a resume file if present */
#ifdef ENABLE_CHECKSUM
	init_chunks(file);
#endif
}

void free_chunks(metalink_file_list_t *file)
{
	chunk_t *tmp, *elem;

	if (!file)
		return;

	elem = file->chunk;

	while (elem) {
		tmp = elem;
		elem = elem->next;
		m_free(tmp);
	}
	file->chunk = NULL;
}
