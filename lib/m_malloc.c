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

#include "m_malloc.h"

#ifdef MULKDEBUG

#define HASH_TABLE_SIZE 131072 /* 2^17, use always power of 2 */
#define HASH_CHUNK_SIZE 2048 
#define INC_INDEX(i) i = ((i == HASH_TABLE_SIZE-1) ? 0 : i+1)


static struct {
  const void *ptr;
  const char *filename;
  int line;
  size_t size;
} hash_table[HASH_TABLE_SIZE];

static int add_count = 0, remove_count = 0;

/* Robert Jenkins' hash function */
/* it works quite well considering at least the 17 lowest bits */
static uint32_t hash_integer(uint32_t a)
{
	a = a ^ (a>>4);
	a = (a^0xdeadbeef) + (a<<5);
	a = a ^ (a>>11);

	return a;
}

/* it takes the lowest 17 bits, if you modify this number of bits, HASH_TABLE_SIZE 
 * has to be chosen accordingly */
static int hash_value(const void *ptr)
{
	uint32_t key = hash_integer((uint32_t) ptr);

	return (int) (key&0x0001FFFF);
}

static int find_pointer(const void *ptr)
{
	int i, pos = hash_value(ptr);

	/* at least one NULL element */
	for (i = pos; hash_table[i].ptr != NULL && hash_table[i].ptr != ptr; INC_INDEX(i));

	return i;
}

static int is_pointer_swap(int hash_ptr, int null_pos, int pos)
{
	if (pos >= hash_ptr) 
		return (null_pos >= hash_ptr && pos > null_pos);
	else 
		return (null_pos >= hash_ptr || pos > null_pos);
}

void m_debug_init(void)
{
	int i;

	for (i = 0; i < HASH_TABLE_SIZE; ++i)
		hash_table[i].ptr = NULL;
}

static mulk_type_return_t add_pointer(const void *ptr, const char *filename, int line, size_t tot_size)
{
	int pos;

	if (add_count - remove_count >= HASH_TABLE_SIZE-1) {
		fprintf(stderr, _("ERROR: increase HASH_TABLE_SIZE to a larger value and recompile\n"));
		exit(EXIT_FAILURE);
	}

	pos = find_pointer(ptr);
	if (hash_table[pos].ptr != NULL)
		return MULK_RET_ERR;

	hash_table[pos].ptr = ptr;
	hash_table[pos].filename = filename;
	hash_table[pos].line = line;
	hash_table[pos].size = tot_size;
	add_count++;

	return MULK_RET_OK;
}

static mulk_type_return_t remove_pointer(const void *ptr)
{
	int i, pos = find_pointer(ptr);

	if (hash_table[pos].ptr == NULL) 
		return MULK_RET_ERR;

	hash_table[pos].ptr = NULL;
	remove_count++;

	for (i = pos, INC_INDEX(i); hash_table[i].ptr != NULL; INC_INDEX(i))
	{
		int hash_ptr = hash_value(hash_table[i].ptr);

		if (is_pointer_swap(hash_ptr, pos, i)) {
			hash_table[pos] = hash_table[i];
			hash_table[i].ptr = NULL;
			pos = i;
		}
	}

	return MULK_RET_OK;
}

void m_print_allocated_pointers(void)
{
	int i;

	printf(_("\nCAlloc:  %d\n"), add_count);
	printf(_("Free:    %d\n"), remove_count);
	printf(_("Memory Leaks: %d\n\n"), add_count - remove_count);

	for (i = 0; i < HASH_TABLE_SIZE; ++i)
		if (hash_table[i].ptr != NULL)
			printf(_("ptr=0x%lx, %s:%d, %ld bytes\n"), (unsigned long) hash_table[i].ptr,
				hash_table[i].filename, hash_table[i].line, (long int) hash_table[i].size);
}

void m_print_pointers_distribution(void)
{
	int i, count = 0;

	for (i = 0; i < HASH_TABLE_SIZE; ++i) {
		if (((i + 1) % HASH_CHUNK_SIZE) == 0) {
			printf(_("chunck %%%d, number %d\n"), (i + 1) / HASH_CHUNK_SIZE, count);
			count = 0;
		}
		if (hash_table[i].ptr != NULL)
			count++;
	}
}

void *debug_calloc(size_t nmemb, size_t size, const char *source_filename, int source_line)
{
	void *ptr = default_calloc(nmemb, size);

	if (add_pointer(ptr, source_filename, source_line, nmemb * size) != MULK_RET_OK) {
		fprintf(stderr, _("ERROR: pointer to add already present at %s:%d\n"), source_filename, source_line);
		exit(EXIT_FAILURE);
	}

	return ptr;
}

void debug_free(void *ptr, const char *source_filename, int source_line)
{
	if (!ptr) {
		fprintf(stderr, _("ERROR: free NULL pointer at %s:%d\n"), source_filename, source_line);
		exit(EXIT_FAILURE);
	}

	if (remove_pointer(ptr) != MULK_RET_OK) {
		fprintf(stderr, _("ERROR: bad free (0x%lx) at %s:%d\n"), (unsigned long) ptr, source_filename, source_line);
		exit(EXIT_FAILURE);
	}

	default_free(ptr);
}

#endif /* MULKDEBUG */

void *default_calloc(size_t nmemb, size_t size)
{
	void *ptr = calloc(nmemb, size);

	if (!ptr) {
		fprintf(stderr, _("ERROR: %s,%d: memory allocation failed (%ld bytes)\n"), __FILE__, __LINE__, (long int) size);
		exit(EXIT_FAILURE);
	}

	return ptr;
}

