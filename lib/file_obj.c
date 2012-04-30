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

#include "file_obj.h"
#include "string_obj.h"
#include "url_list.h"
#include "option_obj.h"

#define MAX_LINE_LENGTH 1000
#define BUFFER_SIZE 1024


int copy(const char *from_filename, const char *to_filename)
{
	char buffer[BUFFER_SIZE];
	FILE *fp_to, *fp_from;
	int ret = -1;
	size_t count;

	if ((fp_from = fopen(from_filename, "rb")) == NULL)
		return -1;

	if ((fp_to = fopen(to_filename, "wb")) == NULL)
		goto Exit1;

	while ((count = fread(buffer, 1, BUFFER_SIZE, fp_from)) != 0)
		if (fwrite(buffer, 1, count, fp_to) < count)
			goto Exit;

	ret = (ferror(fp_to) || ferror(fp_from)) ? -1 : 0;

Exit:
	fclose(fp_to);
Exit1:
	fclose(fp_from);

	return ret;
}

size_t fwrite_offset(unsigned char *buf, size_t size, size_t nmemb, off_t offset, FILE *file)
{
	mulk_fseek(file, offset, SEEK_SET); 

	return fwrite(buf, size, nmemb, file);
}

/* read only long option */
mulk_type_return_t read_option_from_text_file(const char *filename)
{
	FILE *file;
	char buf[MAX_LINE_LENGTH], *option, *value;
	int option_index = 0;
	mulk_type_return_t ret = MULK_RET_OK;

	if ((file = fopen(filename, "r")) == NULL)
		return MULK_RET_FILE_ERR;

	for (;;) {
		if (!fgets(buf, MAX_LINE_LENGTH, file))
			break;
		if (feof(file))
			break;

		string_trim(buf);

		/* skip empty lines and comments */
		if (!strlen(buf) || *buf == '#')
			continue;

		option = buf;
		if ((value = strchr(buf, '=')) != NULL) {
			*value = 0;
			value++;
			if (!*value)
				value = NULL;
		}
		else
			value = NULL;

		MULK_NOTE((_("Add option coming from text file: %s%s%s\n"), option,
			value ? "=" : "", value ? value : ""));

		if ((ret = mulk_find_long_option(option, &option_index)) != MULK_RET_OK)
			break;
	
		if (!strcmp(option, OPT_OPTION_FILE)) {
			fprintf(stderr, _("\nERROR: option not valid inside an option file\n\n"));
			ret = MULK_RET_OPTION_ERR;
			break;
		}

		if ((ret = mulk_set_option(option_index, value)) != MULK_RET_OK)
			break;
	}

	fclose(file);

	return ret;
}

mulk_type_return_t read_uri_from_text_file(const char *filename)
{
	FILE *file;
	char buf[MAX_LINE_LENGTH];
	mulk_type_return_t ret = MULK_RET_OK;

	if ((file = fopen(filename, "r")) == NULL)
		return MULK_RET_FILE_ERR;

	for (;;) {
		if (!fgets(buf, MAX_LINE_LENGTH, file))
			break;
		if (feof(file))
			break;

		string_trim(buf);

		/* skip empty lines and comments */
		if (!strlen(buf) || *buf == '#')
			continue;

		MULK_NOTE((_("Add url to download coming from text file: %s\n"), buf));
		if ((ret = mulk_add_new_url(buf)) != MULK_RET_OK)
			break;
	}

	fclose(file);

	return ret;
}

#ifdef ENABLE_METALINK
mulk_type_return_t read_metalink_list_from_text_file(const char *filename)
{
	FILE *file;
	char buf[MAX_LINE_LENGTH];
	mulk_type_return_t ret = MULK_RET_OK;

	if ((file = fopen(filename, "r")) == NULL)
		return MULK_RET_FILE_ERR;

	for (;;) {
		if (!fgets(buf, MAX_LINE_LENGTH, file))
			break;
		if (feof(file))
			break;

		string_trim(buf);

		/* skip empty lines and comments */
		if (!strlen(buf) || *buf == '#')
			continue;

		MULK_NOTE((_("Add Metalink file to download coming from text file: %s\n"), buf));
		if ((ret = mulk_add_new_metalink_file(buf)) != MULK_RET_OK)
			break;
	}

	fclose(file);

	return ret;
}
#endif /* ENABLE_METALINK */

mulk_type_return_t save_file_to_outputdir(char *oldfilename, char* newfilename, int make_copy)
{
	int res;

	MULK_INFO((_("Saving file: %s\n"), newfilename));

	if (make_dir_pathname(newfilename)) {
		MULK_ERROR((_("ERROR: creating directory error no. %d\n"), errno));

		return MULK_RET_FILE_ERR;
	}

	if (make_copy)
		res = copy(oldfilename, newfilename);
	else
		res = rename(oldfilename, newfilename);

	if (res) {
		/* file exists */
		if (errno != 17)
			MULK_ERROR((_("ERROR: saving file error no. %d\n"), errno));

		remove(oldfilename);

		return MULK_RET_FILE_ERR;
	}

	return MULK_RET_OK;
}

#ifdef _WIN32

int make_dir_pathname(const char *pathname)
{
	struct _stat stat_buf;
	char *slash_ptr;
	char *path = NULL;

	if (!pathname || !*pathname)
		return -1;

	path = string_new(pathname);
	if ((slash_ptr = strchr(path, ':')) != NULL)
		slash_ptr += 2;
	else
		slash_ptr = path;

	slash_ptr = strchr(slash_ptr, *DIR_SEPAR_STR);

	while (slash_ptr) {
		*slash_ptr = 0;

		if (_stat(path, &stat_buf) < 0) {
			if (_mkdir(path) < 0) {
				string_free(path);
				return -1;
			}
		}
		else if (!(stat_buf.st_mode & _S_IFDIR)) {
			string_free(path);
			return -1;
		}

		*slash_ptr = *DIR_SEPAR_STR;
		slash_ptr = strchr(slash_ptr + 1, *DIR_SEPAR_STR);
	}

	string_free(path);
	return 0;
}

int is_file_exist(const char *filename)
{
	struct _stat sts;

	return !(_stat(filename, &sts) == -1 && errno == ENOENT);
}

int create_truncated_file(const char *filename, off_t size)
{
	int file, result = 0;

	if (!_sopen_s(&file, filename, _O_RDWR | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE)) {
		result = _chsize(file, size);
		_close(file);
	}

	return result;
}

int execute_filter(const char *command, char **url, int level)
{
	FILE *fp;
	char buffer[BUFFER_SIZE];
	char *command_line = NULL;
	char *new_url = NULL;

	if (!url || !*url || !**url || !command || !*command)
		return -1;

	string_printf(&command_line, "%s \"%s\" %d", command, *url, level);

	if ((fp = _popen(command_line, "rt")) == NULL) {
		MULK_ERROR((_("ERROR: executing external filter program %s\n"), command));
		return -1;
	}

	new_url = string_alloc(0);
	while (!feof(fp)) {
		if (fgets(buffer, BUFFER_SIZE, fp) != NULL)
			string_cat(&new_url, buffer);
	}
	string_trim(new_url);

	string_free(*url);
	*url = new_url;

	_pclose(fp);
	string_free(command_line);

	return 0;
}

#else /* not _WIN32 */

int make_dir_pathname(const char *pathname)
{
	struct stat stat_buf;
	char *slash_ptr;
	char *path = NULL;
	int arg_mode = DEFAULT_MASK;

	if (!pathname || !*pathname)
		return -1;

	path = string_new(pathname);

	arg_mode |= (S_IWUSR | S_IXUSR);

	slash_ptr = strchr(path, *DIR_SEPAR_STR);

	while (slash_ptr) {
		*slash_ptr = 0;

		if (stat(path, &stat_buf) < 0) {
			if (mkdir(path, arg_mode) < 0) {
				string_free(path);
				return -1;
			}
		}
		else if (!S_ISDIR(stat_buf.st_mode)) {
			string_free(path);
			return -1;
		}

		*slash_ptr = *DIR_SEPAR_STR;
		slash_ptr = strchr(slash_ptr + 1, *DIR_SEPAR_STR);
	}

	string_free(path);
	return 0;
}

int is_file_exist(const char *filename)
{
	struct stat sts;

	return !(stat(filename, &sts) == -1 && errno == ENOENT);
}

int create_truncated_file(const char *filename, off_t size)
{
	return truncate(filename, size);
}

int execute_filter(const char *command, char **url, int level)
{
	FILE *fp;
	char buffer[BUFFER_SIZE];
	char *command_line = NULL;
	char *new_url = NULL;

	if (!url || !*url || !**url || !command || !*command)
		return -1;

	string_printf(&command_line, "%s \"%s\" %d", command, *url, level);

	if ((fp = popen(command_line, "r")) == NULL) {
		MULK_ERROR((_("ERROR: executing external filter program %s\n"), command));
		return -1;
	}

	new_url = string_alloc(0);
	while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
		string_cat(&new_url, buffer);
	}
	string_trim(new_url);

	string_free(*url);
	*url = new_url;

	pclose(fp);
	string_free(command_line);

	return 0;
}

#endif /* not _WIN32 */

