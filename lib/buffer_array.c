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

#include <stdio.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include "buffer_array.h"
#include "jpg_obj.h"
#include "gif_obj.h"
#include "png_obj.h"
#include "url_list.h"
#include "file_obj.h"
#include "string_obj.h"
#include "mime_types.h"
#include "parse.h"
#ifdef ENABLE_CHECKSUM
#include "checksum.h"
#endif
#ifdef ENABLE_METALINK
#include "chunk_list.h"
#endif

buffer_t *buffer_array = NULL;

void reset_buffer_array(void)
{
	buffer_array = m_calloc(option_values.max_sim_conns, sizeof(buffer_t));
}

int get_buffer(CURL *id)
{
	int i;

	for (i = 0; buffer_array[i].id != id && i < option_values.max_sim_conns; i++);

	if (i >= option_values.max_sim_conns)
		return MULK_RET_ERR;

	return i;
}

static int get_buffer_by_url(url_list_t *url, int id)
{
	int i;

	if (!url)
		return MULK_RET_ERR;


	for (i = 0; i < option_values.max_sim_conns; i++)
		if (i != id && buffer_array[i].url && buffer_array[i].url->id == url->id)
			break;

	if (i >= option_values.max_sim_conns)
		return MULK_RET_ERR;

	return i;
}

int count_number_of_hostname(UriUriA *uri)
{
	int i, count = 0;

	if (!uri)
		return MULK_RET_ERR;

	for (i = 0; i < option_values.max_sim_conns; i++)
		if (buffer_array[i].uri && are_hosts_equal(buffer_array[i].uri, uri))
			count++;

	return count;
}

int is_uri_compatible(UriUriA *uri, int maxconns)
{
	int count = count_number_of_hostname(uri);

	return count >= 0 && count < (maxconns > 0 ? maxconns : option_values.max_sim_conns_per_host);
}

#ifdef ENABLE_METALINK
int count_number_of_chunks(metalink_file_list_t *file)
{
	int i, count = 0;

	if (!file)
		return MULK_RET_ERR;

	for (i = 0; i < option_values.max_sim_conns; i++)
	{
		if (buffer_array[i].chunk && buffer_array[i].chunk->file == file)
			count++;
	}

	return count;
}

static mulk_type_return_t init_buffer_file(buffer_t *buffer, metalink_file_list_t *metalink_file)
{
	if (buffer->file_pt)
		fclose(buffer->file_pt);

	if (!metalink_file)
		return MULK_RET_ERR;

	if (metalink_file->resume_filename && is_file_exist(metalink_file->resume_filename))
		rename(metalink_file->resume_filename, buffer->filename);

	if (create_truncated_file(buffer->filename, metalink_file->size))
		return MULK_RET_FILE_ERR;

	if (!(buffer->file_pt = fopen(buffer->filename, "rb+")))
		return MULK_RET_FILE_ERR;

	buffer->url->tmp_file_created = 1;

	return MULK_RET_OK;
}

int open_buffer(CURL *id, url_list_t *url, UriUriA *uri, chunk_t *chunk, metalink_resource_list_t *resource,
	int header)
#else /* not ENABLE_METALINK */
int open_buffer(CURL *id, url_list_t *url, UriUriA *uri)
#endif
{
	int i;

	if ((i = get_buffer(NULL)) < 0)
		return i;

	buffer_array[i].id = id;
	buffer_array[i].url = url;
	buffer_array[i].uri = uri;
#ifdef ENABLE_METALINK
	buffer_array[i].chunk = chunk;
	buffer_array[i].used_res = resource;

	if (chunk || header) {
		int id_file;

		string_printf(&buffer_array[i].filename, "%smetalink-mulktmp%05d",
			option_values.temp_directory, url->id);

		if (header || !is_file_exist(buffer_array[i].filename) || !url->tmp_file_created) {
			if (!make_dir_pathname(buffer_array[i].filename)) {
				if (is_file_exist(buffer_array[i].filename))
					buffer_array[i].file_pt = fopen(buffer_array[i].filename, "rb+");
				else
					buffer_array[i].file_pt = fopen(buffer_array[i].filename, "wb");
				if (!header && url->metalink_uri->size > 0)
					init_buffer_file(&buffer_array[i], url->metalink_uri);
			}
		}
		else if ((id_file = get_buffer_by_url(url, i)) < 0) {
			if (!make_dir_pathname(buffer_array[i].filename))
				buffer_array[i].file_pt = fopen(buffer_array[i].filename, "rb+");
		}
		else {
			buffer_array[i].file_pt = buffer_array[id_file].file_pt;
		}
	}
	else
#endif /* ENABLE_METALINK */
	{
		string_printf(&buffer_array[i].filename, "%smulktmp%05d",
			option_values.temp_directory, i);

		if (!make_dir_pathname(buffer_array[i].filename))
			buffer_array[i].file_pt = fopen(buffer_array[i].filename, "wb");
	}

	if (is_printf(MINFO)) {
		char *uri_str = uri2string(uri);
		MULK_INFO((_("Open link #%d, url: %s, tmp file: %s\n"), i, uri_str ? uri_str : "",
			buffer_array[i].filename));
		string_free(&uri_str);
	}

	return i;
}

static mulk_type_return_t filter_buffer(int i, int valid_res, const char *base_url, long err_code, long resp_code)
{
	char *newfilename = NULL;
	char *newmimefilename = NULL;
	char *subtype = NULL;
	char *type = NULL;
	int len;
	mulk_type_return_t ret = MULK_RET_OK;

#ifdef ENABLE_METALINK
	if (buffer_array[i].url->metalink_uri)
		buffer_array[i].url->http_code = 0;
	else
#endif
	{
		buffer_array[i].url->err_code = err_code;
		buffer_array[i].url->http_code = resp_code;
	}

	if (!valid_res) {
#ifdef ENABLE_METALINK
		if (buffer_array[i].url->metalink_uri)
			buffer_array[i].url->err_code = METALINK_RES_NO_NORE_RESOURCES;
#endif
		remove(buffer_array[i].filename);
		return MULK_RET_ERR;
	}

#ifdef ENABLE_RECURSION
	if (is_html_file(buffer_array[i].url->mimetype)) 
		parse_urls(buffer_array[i].filename, base_url, buffer_array[i].url->level);
#endif

#ifdef ENABLE_METALINK
	if (option_values.follow_metalink && is_metalink_file(buffer_array[i].url->mimetype)) 
		add_new_metalink(buffer_array[i].filename, buffer_array[i].url->level);

	if (buffer_array[i].url->metalink_uri) {
#ifdef ENABLE_CHECKSUM
		if (verify_metalink_file(buffer_array[i].url->metalink_uri->file, buffer_array[i].filename) == CS_VERIFY_ERR) {
			MULK_NOTE((_("The file will be deleted.\n")));
			buffer_array[i].url->err_code = METALINK_RES_WRONG_CHECKSUM;

			ret = MULK_RET_ERR;
		}
		else
#endif /* ENABLE_CHECKSUM */
		{
			string_printf(&newfilename, "%s%s", option_values.file_output_directory,
				buffer_array[i].url->metalink_uri->file->name);
			buffer_array[i].url->err_code = METALINK_RES_OK;
		}
	}
	else 
#endif /* ENABLE_METALINK */
	if (!option_values.disable_save_tree) {
		UriUriA *uri = create_absolute_uri(NULL, base_url);
		char *furi_str = uri2filename(uri);

		if (furi_str) {
			string_printf(&newfilename, "%s%s", option_values.file_output_directory, furi_str);
			len = strlen(newfilename);

			/* add index.<mime-type> if uri doesn't contains an explicit path */
			if ((!uri->pathHead || !uri->pathHead->text.afterLast || !uri->pathHead->text.first
				|| !(uri->pathHead->text.afterLast-uri->pathHead->text.first))
				&& newfilename[len-1] != *DIR_SEPAR_STR) {
				string_cat(&newfilename, DIR_SEPAR_STR);
				len++;
			}

			/* add index.<mime-type> if the filename represents a directory name */
			if (newfilename[len-1] == *DIR_SEPAR_STR) {
				string_cat(&newfilename, "index.");
				if (extract_mime_type(buffer_array[i].url->mimetype, NULL, &subtype) == MULK_RET_OK) {
					string_cat(&newfilename, subtype);
					string_free(&subtype);
				} else
					string_cat(&newfilename, "bin");
			}
		}

		string_free(&furi_str);
		uri_free(uri);
	}

	if ((is_gif_image(buffer_array[i].url->mimetype) && is_valid_gif_image(buffer_array[i].filename))
		|| (is_png_image(buffer_array[i].url->mimetype) && is_valid_png_image(buffer_array[i].filename)) 
		|| (is_jpeg_image(buffer_array[i].url->mimetype) && is_valid_jpeg_image(buffer_array[i].filename))
		|| (is_saved_mime_type(buffer_array[i].url->mimetype))) {
		if (extract_mime_type(buffer_array[i].url->mimetype, &type, &subtype) == MULK_RET_OK) {
			string_printf(&newmimefilename, "%s%s%s%s_%05d.%s", option_values.mime_output_directory,
				type, DIR_SEPAR_STR, subtype, buffer_array[i].url->id, subtype);
			string_free(&type);
			string_free(&subtype);
		}
	}

	/* save file */
	buffer_array[i].url->filename = string_new(newfilename);
	buffer_array[i].url->mimefilename = string_new(newmimefilename);

	if (newfilename && newmimefilename) {
		if ((ret = save_file_to_outputdir(buffer_array[i].filename, newfilename, 1)) == MULK_RET_OK)
			ret = save_file_to_outputdir(buffer_array[i].filename, newmimefilename, 0);
	}
	else if (newfilename) 
		ret = save_file_to_outputdir(buffer_array[i].filename, newfilename, 0);
	else if (newmimefilename) 
		ret = save_file_to_outputdir(buffer_array[i].filename, newmimefilename, 0);
	else
		remove(buffer_array[i].filename);

	string_free(&newmimefilename);
	string_free(&newfilename);

	return ret;
}

int is_uri_http_buffer(CURL *id)
{
	int i;

	if ((i = get_buffer(id)) < 0)
		return 0;

	if (is_uri_http(buffer_array[i].uri))
		return 1;

	return 0;
}

void set_buffer_mime_type(CURL *id, const char *mimetype)
{
	int i;
	char *ptr;

	if ((i = get_buffer(id)) < 0)
		return;

	string_free(&buffer_array[i].url->mimetype);
	
	if (mimetype) {
		buffer_array[i].url->mimetype = string_new(mimetype);
		if ((ptr = strchr(buffer_array[i].url->mimetype, ';')) != NULL)
			*ptr = 0;
		string_lower(buffer_array[i].url->mimetype);
	}
}

#ifdef ENABLE_METALINK
static int is_valid_response(UriUriA *uri, CURLcode err_code, long resp_code, chunk_t *chunk, int single_chunk)
#else /* not ENABLE_METALINK */
static int is_valid_response(UriUriA *uri, CURLcode err_code, long resp_code)
#endif /* not ENABLE_METALINK */
{
	if (err_code)
		return 0;

#ifdef ENABLE_METALINK
	if (chunk) {
		if (is_uri_http(uri))
			return (resp_code == 206 || (resp_code == 200 && single_chunk)) && is_chunk_downloaded(chunk);
		else
			return is_chunk_downloaded(chunk);
	}
#endif /* ENABLE_METALINK */

	if (is_uri_http(uri))
		return resp_code == 200;

	return 1;
}

mulk_type_return_t close_buffer(CURL *id, const char *base_url, CURLcode err_code, long resp_code, int *file_completed)
{
#ifdef ENABLE_METALINK
	double length_double;
	long length, byte_downloaded = 0, byte_total = 0;
	int chunk_completed = 0, chunk_total = 0, single_chunk = 0, is_file_ok = 0;
#endif
	int i, valid_res = 0;
	mulk_type_return_t ret = MULK_RET_OK;

	if ((i = get_buffer(id)) < 0)
		return MULK_RET_ERR;

	if (file_completed)
		*file_completed = 0;

	MULK_INFO((_("Close link #%d\n"), i));

#ifdef ENABLE_METALINK
	if (buffer_array[i].url->metalink_uri) {
		buffer_array[i].url->metalink_uri->header = 0;

		if (buffer_array[i].url->metalink_uri->size < 0) {
			valid_res = is_valid_response(buffer_array[i].uri, err_code, resp_code, NULL, 0);

		   	if (valid_res) {
				curl_easy_getinfo(id, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length_double);
				length = (long) length_double;

				if (length > 0) {
					buffer_array[i].url->metalink_uri->size = length;

					goto Exit;
				}
				else
					valid_res = 0;
			}
		}
		else {
			single_chunk = buffer_array[i].chunk && (buffer_array[i].url->metalink_uri->chunk_number == 1);
			valid_res = is_valid_response(buffer_array[i].uri, err_code, resp_code, buffer_array[i].chunk, single_chunk);
		}

		/* actual URL no more used, decrement assignement counter */
		if (buffer_array[i].used_res) {
			buffer_array[i].used_res->assigned--;
			if (buffer_array[i].used_res->assigned < 0)
				buffer_array[i].used_res->assigned = 0;
		}

		if (buffer_array[i].chunk) {
			buffer_array[i].chunk->used_res = NULL;

			if (!valid_res)
				reset_chunk(buffer_array[i].chunk);

			if (file_statistics(buffer_array[i].url->metalink_uri, &chunk_completed, &chunk_total,
				&byte_downloaded, &byte_total) == MULK_RET_OK) {
				MULK_INFO((_("Downloaded %d/%d chunks, %ld/%ld bytes\n"), chunk_completed, chunk_total,
					byte_downloaded, byte_total));
			}

			is_file_ok = (chunk_completed == chunk_total);
		}

		/* remove URL that returns an error from the list of usable URLs */
		if (!valid_res && buffer_array[i].used_res)
			remove_metalink_resource(buffer_array[i].url->metalink_uri, buffer_array[i].used_res);

		if (buffer_array[i].chunk && is_file_ok) 
			MULK_NOTE((_("RESULT: Metalink downloaded successfully, Filename:\"%s\" Size:%ld\n"),
				buffer_array[i].url->metalink_uri->file->name, byte_downloaded));
		else if (!is_resource_available(buffer_array[i].url->metalink_uri, 
				(buffer_array[i].url->metalink_uri->size < 0)))
			MULK_NOTE((_("RESULT: Metalink error, no more usable URLs for downloading, Filename:\"%s\"\n"),
				buffer_array[i].url->metalink_uri->file->name));
	}
	else
		valid_res = is_valid_response(buffer_array[i].uri, err_code, resp_code, NULL, 0);
#else /* not ENABLE_METALINK */
	valid_res = is_valid_response(buffer_array[i].uri, err_code, resp_code);
#endif /* ENABLE_METALINK */

	/* is the last chunk? */
	if (get_buffer_by_url(buffer_array[i].url, i) < 0) {
#ifdef ENABLE_METALINK
		if (buffer_array[i].url->metalink_uri) {
			if (!is_file_ok && is_resource_available(buffer_array[i].url->metalink_uri,
					(buffer_array[i].url->metalink_uri->size < 0)))
				goto Exit;

			valid_res = is_file_ok;
		}
#endif

		if (buffer_array[i].file_pt)
			fclose(buffer_array[i].file_pt);
		buffer_array[i].file_pt = NULL;

		if ((ret = filter_buffer(i, valid_res, base_url, err_code, resp_code)) == MULK_RET_OK && file_completed)
			*file_completed = 1;
	}
	
#ifdef ENABLE_METALINK
Exit:
	buffer_array[i].chunk = NULL;
	buffer_array[i].used_res = NULL;
#endif
	buffer_array[i].file_pt = NULL;
	buffer_array[i].id = NULL;
	buffer_array[i].url = NULL;
	buffer_array[i].uri = NULL;
	string_free(&buffer_array[i].filename);

	return ret;
}

void print_buffers(void)
{
	int i;

	if (is_printf(MINFO)) {
		for (i = 0; i < option_values.max_sim_conns; i++)
		{
#ifdef ENABLE_METALINK
			char *uri_str = uri2string(buffer_array[i].used_res->uri);
			MULK_INFO(("%d: %s, %s, %s", i, buffer_array[i].id ? _("PRESENT") : _("EMPTY"), 
				buffer_array[i].used_res ? uri_str : _("NULL"),
				buffer_array[i].chunk ? _("CHUNK") : _("NO CHUNK")));
			string_free(&uri_str);
#else /* not ENABLE_METALINK */
			MULK_INFO(("%d: %s", i, buffer_array[i].id ? _("PRESENT") : _("EMPTY")));
#endif /* not ENABLE_METALINK */
			MULK_INFO(("\n"));
		} 
	}
}

void free_buffer_easy_handles(CURLM *curl_obj)
{
	int i;
	char *orig_url = NULL;

	if (!buffer_array)
		return;

	for (i = 0; i < option_values.max_sim_conns; i++) {
		if (!buffer_array[i].id)
			continue;

		curl_easy_getinfo(buffer_array[i].id, CURLINFO_PRIVATE, &orig_url);
		curl_multi_remove_handle(curl_obj, buffer_array[i].id);
		string_free(&orig_url);
		curl_easy_cleanup(buffer_array[i].id);

		buffer_array[i].id = NULL;
	}
}

void free_buffer_array(void)
{
	int i;

	if (!buffer_array)
		return;

	for (i = 0; i < option_values.max_sim_conns; i++)
		string_free(&buffer_array[i].filename);

	m_free(buffer_array);

	if (is_file_exist(option_values.temp_directory) && remove_dir(option_values.temp_directory))
		MULK_ERROR((_("ERROR: removing temporary directory\n")));
}

