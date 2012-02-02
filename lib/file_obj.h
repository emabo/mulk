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

#ifndef _FILE_OBJ_H_
#define _FILE_OBJ_H_

#include "defines.h"
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <share.h>
#else
#include <unistd.h>
#endif

#define DEFAULT_MASK 0777

#ifdef _WIN32
#define remove_dir _rmdir
#else
#define remove_dir rmdir
#endif

int make_dir_pathname(const char *pathname);

int copy(const char *from_filename, const char *to_filename);

size_t fwrite_offset(unsigned char *buf, size_t size, size_t nmemb, off_t offset, FILE *file);

int is_file_exist(const char *filename);

mulk_type_return_t read_option_from_text_file(const char *filename);
mulk_type_return_t read_uri_from_text_file(const char *filename);
#ifdef ENABLE_METALINK
mulk_type_return_t read_metalink_list_from_text_file(const char *filename);
#endif

mulk_type_return_t save_file_to_outputdir(char *oldfilename, char* newfilename, int copy);

int create_truncated_file(const char *filename, off_t size);

int execute_filter(const char *command, char **url, int level);

#endif /* not _FILE_OBJ_H_ */
