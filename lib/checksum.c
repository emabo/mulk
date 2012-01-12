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

#include "checksum.h"
#include "string_obj.h"

#define FILE_BLOCK 4096

#ifdef HAVE_OPENSSL_MD2
#define CS_MD2_STR "md2"
#endif
#define CS_MD4_STR "md4"
#define CS_MD5_STR "md5"
#define CS_SHA1_STR "sha1"
#define CS_SHA224_STR "sha224"
#define CS_SHA256_STR "sha256"
#define CS_SHA384_STR "sha384"
#define CS_SHA512_STR "sha512"


static val_str_t checksum_strings[] = {
#ifdef HAVE_OPENSSL_MD2
	{CS_MD2, CS_MD2_STR},
#endif
	{CS_MD4, CS_MD4_STR},
	{CS_MD5, CS_MD5_STR},
	{CS_SHA1, CS_SHA1_STR},
	{CS_SHA224, CS_SHA224_STR},
	{CS_SHA256, CS_SHA256_STR},
	{CS_SHA384, CS_SHA384_STR},
	{CS_SHA512, CS_SHA512_STR},
	{-1, NULL}
};

void init_context(checksum_t *cs)
{
	if (!cs)
		return;

	switch (cs->cs_type) {
#ifdef HAVE_OPENSSL_MD2
		case CS_MD2:
			MD2_Init(&(cs->context.md2));
			break;
#endif
		case CS_MD4:
			MD4_Init(&(cs->context.md4));
			break;
		case CS_MD5:
			MD5_Init(&(cs->context.md5));
			break;
		case CS_SHA1:
			SHA1_Init(&(cs->context.sha1));
			break;
		case CS_SHA224:
			SHA224_Init(&(cs->context.sha224));
			break;
		case CS_SHA256:
			SHA256_Init(&(cs->context.sha256));
			break;
		case CS_SHA384:
			SHA384_Init(&(cs->context.sha384));
			break;
		case CS_SHA512:
			SHA512_Init(&(cs->context.sha512));
			break;
		default:
			break;
	}
}

void update_context(checksum_t *cs, const unsigned char *input, size_t length)
{
	if (!cs)
		return;

	switch (cs->cs_type) {
#ifdef HAVE_OPENSSL_MD2
		case CS_MD2:
			MD2_Update(&(cs->context.md2), input, length);
			break;
#endif
		case CS_MD4:
			MD4_Update(&(cs->context.md4), input, length);
			break;
		case CS_MD5:
			MD5_Update(&(cs->context.md5), input, length);
			break;
		case CS_SHA1:
			SHA1_Update(&(cs->context.sha1), input, length);
			break;
		case CS_SHA224:
			SHA224_Update(&(cs->context.sha224), input, length);
			break;
		case CS_SHA256:
			SHA256_Update(&(cs->context.sha256), input, length);
			break;
		case CS_SHA384:
			SHA384_Update(&(cs->context.sha384), input, length);
			break;
		case CS_SHA512:
			SHA512_Update(&(cs->context.sha512), input, length);
			break;
		default:
			break;
	}
}

void final_context(checksum_t *cs)
{
	if (!cs)
		return;

	switch (cs->cs_type) {
#ifdef HAVE_OPENSSL_MD2
		case CS_MD2:
			MD2_Final(cs->digest.md2, &(cs->context.md2));
			break;
#endif
		case CS_MD4:
			MD4_Final(cs->digest.md4, &(cs->context.md4));
			break;
		case CS_MD5:
			MD5_Final(cs->digest.md5, &(cs->context.md5));
			break;
		case CS_SHA1:
			SHA1_Final(cs->digest.sha1, &(cs->context.sha1));
			break;
		case CS_SHA224:
			SHA224_Final(cs->digest.sha224, &(cs->context.sha224));
			break;
		case CS_SHA256:
			SHA256_Final(cs->digest.sha256, &(cs->context.sha256));
			break;
		case CS_SHA384:
			SHA384_Final(cs->digest.sha384, &(cs->context.sha384));
			break;
		case CS_SHA512:
			SHA512_Final(cs->digest.sha512, &(cs->context.sha512));
			break;
		default:
			break;
	}
}

static char *str_hex_digest(const unsigned char *digest_str, int length)
{
	static char key[SHA512_DIGEST_LENGTH*2+1];
	int i, len = 0;

	for (i = 0; i < length; i++)
		len += sprintf(key + len, "%2.2x", digest_str[i]);
	key[len] = 0;

	MULK_NOTE(("%s\n", key));
	return key;
}

char *str_digest(checksum_t *cs)
{
	char *str = NULL;

	if (!cs)
		return NULL;

	switch (cs->cs_type) {
#ifdef HAVE_OPENSSL_MD2
		case CS_MD2:
			str = str_hex_digest(cs->digest.md2, MD2_DIGEST_LENGTH);
			break;
#endif
		case CS_MD4:
			str = str_hex_digest(cs->digest.md4, MD4_DIGEST_LENGTH);
			break;
		case CS_MD5:
			str = str_hex_digest(cs->digest.md5, MD5_DIGEST_LENGTH);
			break;
		case CS_SHA1:
			str = str_hex_digest(cs->digest.sha1, SHA_DIGEST_LENGTH);
			break;
		case CS_SHA224:
			str = str_hex_digest(cs->digest.sha224, SHA224_DIGEST_LENGTH);
			break;
		case CS_SHA256:
			str = str_hex_digest(cs->digest.sha256, SHA256_DIGEST_LENGTH);
			break;
		case CS_SHA384:
			str = str_hex_digest(cs->digest.sha384, SHA384_DIGEST_LENGTH);
			break;
		case CS_SHA512:
			str = str_hex_digest(cs->digest.sha512, SHA512_DIGEST_LENGTH);
			break;
		default:
			break;
	}

	return str;
}

static char *compute_file_checksum(const char *filename, checksum_type_t cs)
{
	FILE *file;
	unsigned char buffer[FILE_BLOCK];
	size_t num;
	checksum_t checksum;

	if (!filename)
		return NULL;

	if (!(file = fopen(filename, "r")))
		return NULL;

	checksum.cs_type = cs;

	init_context(&checksum);
	while (!feof(file))
		if ((num = fread(buffer, 1, FILE_BLOCK, file)))
			update_context(&checksum, buffer, num);

	fclose(file);

	final_context(&checksum);

	return str_digest(&checksum);
}

off_t update_context_chunk_file(FILE *file, checksum_t *checksum, off_t offset, off_t size) 
{
	unsigned char buffer[FILE_BLOCK];
	size_t byte_to_read, num;
	off_t byte_read = 0;

	if (!file || !checksum || size < 0)
		return 0;

	mulk_fseek(file, offset, SEEK_SET); 

	while (size) {
		byte_to_read = (size >= FILE_BLOCK) ? FILE_BLOCK : size;
		size -= byte_to_read;

		if ((num = fread(buffer, 1, byte_to_read, file)))
			update_context(checksum, buffer, num);

		byte_read += num;
	}

	return byte_read;
}

static checksum_verify_type_t verify_checksum(const char *filename, checksum_type_t cs, char *hash)
{
	char *str_checksum;

	/* no checksum to verify */
	if (cs <= CS_NONE || !hash)
		return CS_VERIFY_NONE;

	str_checksum = compute_file_checksum(filename, cs);

	return (str_checksum && !strcmp(str_checksum, hash)) ? CS_VERIFY_OK : CS_VERIFY_ERR;
}

char *checksum_type2string(checksum_type_t cs)
{
	return value2string(checksum_strings, cs);
}

checksum_type_t string2checksum_type(const char *str)
{
	int ret = string2value(checksum_strings, str);

	return ret < 0 ? CS_NONE : ret;
}

static checksum_type_t find_verification_hash(metalink_file_t *file, char **hash)
{
	metalink_checksum_t** checksums;
	checksum_type_t type, max_type = CS_NONE;
	
	if (hash)
		*hash = NULL;

	for (checksums = file->checksums; checksums && *checksums; checksums++) {
		/* checksum not recognised */
		if (!(type = string2checksum_type((*checksums)->type)))
			continue;

		if (type > max_type) {
			max_type = type;
			if (hash)
				*hash = (*checksums)->hash;
		}
	}

	return max_type; 
}

checksum_verify_type_t verify_metalink_file(metalink_file_t *file, const char *filename)
{
	checksum_type_t cs;
	char *hash = NULL;
	checksum_verify_type_t ret;

	if (!file || !filename)
		return CS_VERIFY_ERR;

	if ((cs = find_verification_hash(file, &hash)) <= CS_NONE) {
		MULK_NOTE((_("No compatible checksum to verify.\n")));
		return CS_VERIFY_NONE;
	}

	MULK_NOTE((_("Verifying %s checksum on %s. This may take a while...\n"), 
		checksum_type2string(cs), file->name));

	ret = verify_checksum(filename, cs, hash);
	if (ret == CS_VERIFY_OK)
		MULK_NOTE((_("Checksum correct.\n")));
	else if (ret == CS_VERIFY_ERR)
		MULK_NOTE((_("Checksum wrong.\n")));

	return ret;
}
