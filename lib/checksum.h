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

#ifndef _CHECKSUM_H_
#define _CHECKSUM_H_

#include "defines.h"
#ifdef HAVE_OPENSSL_MD2
#include <openssl/md2.h>
#endif
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <metalink/metalink_parser.h>


typedef enum checksum_verify_type_t {
	CS_VERIFY_ERR = -1,
	CS_VERIFY_NONE = 0,
	CS_VERIFY_OK = 1
} checksum_verify_type_t;

typedef enum checksum_type_t {
	CS_NONE = 0,
#ifdef HAVE_OPENSSL_MD2
	CS_MD2 = 1,
#endif
	CS_MD4 = 2,
	CS_MD5 = 3,
	CS_SHA1 = 4,
	CS_SHA224 = 5,
	CS_SHA256 = 6,
	CS_SHA384 = 7,
	CS_SHA512 = 8
} checksum_type_t;

typedef union context_t {
#ifdef HAVE_OPENSSL_MD2
	MD2_CTX md2;
#endif
	MD4_CTX md4;
	MD5_CTX md5;
	SHA_CTX sha1;
	SHA256_CTX sha224;
	SHA256_CTX sha256;
	SHA512_CTX sha384;
	SHA512_CTX sha512;
} context_t;

typedef union digest_t {
#ifdef HAVE_OPENSSL_MD2
	unsigned char md2[MD2_DIGEST_LENGTH];
#endif
	unsigned char md4[MD4_DIGEST_LENGTH];
	unsigned char md5[MD5_DIGEST_LENGTH];
	unsigned char sha1[SHA_DIGEST_LENGTH];
	unsigned char sha224[SHA224_DIGEST_LENGTH];
	unsigned char sha256[SHA256_DIGEST_LENGTH];
	unsigned char sha384[SHA384_DIGEST_LENGTH];
	unsigned char sha512[SHA512_DIGEST_LENGTH];
} digest_t;

typedef struct checksum_t {
	checksum_type_t cs_type;
	context_t context;
	digest_t digest;
} checksum_t;


checksum_type_t string2checksum_type(const char *str);
char *checksum_type2string(checksum_type_t cs);

void init_context(checksum_t *cs);
void update_context(checksum_t *cs, const unsigned char *input, size_t length);
off_t update_context_chunk_file(FILE *file, checksum_t *checksum, off_t offset, off_t size);
void final_context(checksum_t *cs);
char *str_digest(checksum_t *cs);

checksum_verify_type_t verify_metalink_file(metalink_file_t *file, const char *filename);


#endif 
