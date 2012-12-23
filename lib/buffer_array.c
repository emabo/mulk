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

void create_buffer_array(void)
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
	buffer_t *buffer;

	if ((i = get_buffer(NULL)) < 0)
		return i;

	buffer = buffer_array + i;
	buffer->id = id;
	buffer->url = url;
	buffer->uri = uri;
#ifdef ENABLE_METALINK
	buffer->chunk = chunk;
	buffer->used_res = resource;

	if (chunk || header) {
		int id_file;

		string_printf(&buffer->filename, "%smetalink-mulktmp%05d",
			option_values.temp_directory, url->id);

		if (header || !is_file_exist(buffer->filename) || !url->tmp_file_created) {
			if (!make_dir_pathname(buffer->filename)) {
				if (is_file_exist(buffer->filename))
					buffer->file_pt = fopen(buffer->filename, "rb+");
				else
					buffer->file_pt = fopen(buffer->filename, "wb");
				if (!header && url->metalink_uri->size > 0)
					init_buffer_file(buffer, url->metalink_uri);
			}
		}
		else if ((id_file = get_buffer_by_url(url, i)) < 0) {
			if (!make_dir_pathname(buffer->filename))
				buffer->file_pt = fopen(buffer->filename, "rb+");
		}
		else {
			buffer->file_pt = buffer_array[id_file].file_pt;
		}
	}
	else
#endif /* ENABLE_METALINK */
	{
		string_printf(&buffer->filename, "%smulktmp%05d",
			option_values.temp_directory, i);

		if (!make_dir_pathname(buffer->filename))
			buffer->file_pt = fopen(buffer->filename, "wb");
	}

	if (is_printf(MINFO)) {
		char *uri_str = uri2string(uri);
		MULK_INFO((_("Open link #%d, url: %s, tmp file: %s\n"), i, uri_str ? uri_str : "",
			buffer->filename ? buffer->filename : ""));
		string_free(uri_str);
	}

	return i;
}

static mulk_type_return_t filter_buffer(int i, int valid_res, long err_code, long resp_code)
{
	char *newfilename = NULL;
	char *newmimefilename = NULL;
	char *subtype = NULL;
	char *type = NULL;
	mulk_type_return_t ret = MULK_RET_OK;
	buffer_t *buffer = buffer_array + i;

#ifdef ENABLE_METALINK
	metalink_file_list_t *metalink = buffer->url->metalink_uri;

	if (metalink)
		buffer->url->http_code = 0;
	else
#endif
	{
		buffer->url->err_code = err_code;
		buffer->url->http_code = resp_code;
	}

	if (!valid_res) {
#ifdef ENABLE_METALINK
		if (metalink)
			buffer->url->err_code = METALINK_RES_NO_NORE_RESOURCES;
#endif
		remove(buffer->filename);
		return MULK_RET_ERR;
	}

#ifdef ENABLE_RECURSION
	if (is_html_file(buffer->url->mimetype)) 
		parse_urls(buffer->filename, buffer->url);
#endif

#ifdef ENABLE_METALINK
	if (option_values.follow_metalink && is_metalink_file(buffer->url->mimetype)) 
		add_new_metalink(buffer->filename, buffer->url->level);

	if (metalink) {
#ifdef ENABLE_CHECKSUM
		if (verify_metalink_file(metalink->file, buffer->filename) == CS_VERIFY_ERR) {
			MULK_NOTE((_("The file will be deleted.\n")));
			buffer->url->err_code = METALINK_RES_WRONG_CHECKSUM;

			ret = MULK_RET_ERR;
		}
		else
#endif /* ENABLE_CHECKSUM */
		{
			newfilename = string_new(buffer->url->filename);
			buffer->url->err_code = METALINK_RES_OK;
		}
	}
	else 
#endif /* ENABLE_METALINK */
	if (!option_values.disable_save_tree)
		newfilename = string_new(buffer->url->filename);

	if ((is_gif_image(buffer->url->mimetype) && is_valid_gif_image(buffer->filename))
		|| (is_png_image(buffer->url->mimetype) && is_valid_png_image(buffer->filename)) 
		|| (is_jpeg_image(buffer->url->mimetype) && is_valid_jpeg_image(buffer->filename))
		|| (is_saved_mime_type(buffer->url->mimetype))) {
		if (extract_mime_type(buffer->url->mimetype, &type, &subtype) == MULK_RET_OK) {
			string_printf(&newmimefilename, "%s%s%s%s_%05d.%s", option_values.mime_output_directory,
				type, DIR_SEPAR_STR, subtype, buffer->url->id, subtype);
			string_free(type);
			string_free(subtype);
		}
	}

	/* save file */
	buffer->url->mimefilename = string_new(newmimefilename);

	if (newfilename && newmimefilename) {
		if ((ret = save_file_to_outputdir(buffer->filename, newfilename, 1)) == MULK_RET_OK)
			ret = save_file_to_outputdir(buffer->filename, newmimefilename, 0);
	}
	else if (newfilename) 
		ret = save_file_to_outputdir(buffer->filename, newfilename, 0);
	else if (newmimefilename) 
		ret = save_file_to_outputdir(buffer->filename, newmimefilename, 0);
	else
		remove(buffer->filename);

	string_free(newmimefilename);
	string_free(newfilename);

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
	buffer_t *buffer;

	if ((i = get_buffer(id)) < 0)
		return;

	buffer = buffer_array + i;

	string_free(buffer->url->mimetype);
	
	if (mimetype) {
		buffer->url->mimetype = string_new(mimetype);
		if ((ptr = strchr(buffer->url->mimetype, ';')) != NULL)
			*ptr = 0;
		string_lower(buffer->url->mimetype);
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

mulk_type_return_t close_buffer(CURL *id, CURLcode err_code, long resp_code, int *file_completed)
{
#ifdef ENABLE_METALINK
	double length_double;
	off_t length, byte_downloaded = 0, byte_total = 0;
	int chunk_completed = 0, chunk_total = 0, single_chunk = 0, is_file_ok = 0;
	metalink_file_list_t *metalink;
#endif /* ENABLE_METALINK */
	int i, valid_res = 0;
	mulk_type_return_t ret = MULK_RET_OK;
	buffer_t *buffer;

	if ((i = get_buffer(id)) < 0)
		return MULK_RET_ERR;

	if (file_completed)
		*file_completed = 0;

	MULK_INFO((_("Close link #%d\n"), i));

	buffer = buffer_array + i;

#ifdef ENABLE_METALINK
	metalink = buffer->url->metalink_uri;

	if (metalink) {
		metalink->header = 0;

		if (metalink->size < 0) {
			valid_res = is_valid_response(buffer->uri, err_code, resp_code, NULL, 0);

		   	if (valid_res) {
				if (curl_easy_getinfo(id, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length_double) == CURLE_OK)
					length = (off_t) length_double;
				else
					length = 0;

				if (length > 0) {
					metalink->size = length;

					goto Exit;
				}
				else
					valid_res = 0;
			}
		}
		else {
			single_chunk = buffer->chunk && (metalink->chunk_number == 1);
			valid_res = is_valid_response(buffer->uri, err_code, resp_code, buffer->chunk, single_chunk);
		}

		/* actual URL no more used, decrement assignement counter */
		if (buffer->used_res) {
			buffer->used_res->assigned--;
			if (buffer->used_res->assigned < 0)
				buffer->used_res->assigned = 0;
		}

		if (buffer->chunk) {
			buffer->chunk->used_res = NULL;

			if (!valid_res)
				reset_chunk(buffer->chunk);

			if (file_statistics(metalink, &chunk_completed, &chunk_total,
				&byte_downloaded, &byte_total) == MULK_RET_OK) {
				MULK_INFO((_("Downloaded %d/%d chunks, %" PRIdMAX "/%" PRIdMAX " bytes\n"), chunk_completed, chunk_total,
					(intmax_t) byte_downloaded, (intmax_t) byte_total));
			}

			is_file_ok = (chunk_completed == chunk_total);
		}

		/* remove URL that returns an error from the list of usable URLs */
		if (!valid_res && buffer->used_res)
			remove_metalink_resource(metalink, buffer->used_res);

		if (buffer->chunk && is_file_ok)
			MULK_NOTE((_("RESULT: Metalink downloaded successfully, Filename:\"%s\" Size:%" PRIdMAX "\n"),
				metalink->file->name, (intmax_t) byte_downloaded));
		else if (!is_resource_available(metalink, (metalink->size < 0)))
			MULK_NOTE((_("RESULT: Metalink error, no more usable URLs for downloading, Filename:\"%s\"\n"),
				metalink->file->name));
	}
	else
		valid_res = is_valid_response(buffer->uri, err_code, resp_code, NULL, 0);
#else /* not ENABLE_METALINK */
	valid_res = is_valid_response(buffer->uri, err_code, resp_code);
#endif /* ENABLE_METALINK */

	/* is the last chunk? */
	if (get_buffer_by_url(buffer->url, i) < 0) {
#ifdef ENABLE_METALINK
		if (metalink) {
			if (!is_file_ok && is_resource_available(metalink, (metalink->size < 0)))
				goto Exit;

			valid_res = is_file_ok;
		}
#endif /* ENABLE_METALINK */

		if (buffer->file_pt)
			fclose(buffer->file_pt);
		buffer->file_pt = NULL;

		if ((ret = filter_buffer(i, valid_res, err_code, resp_code)) == MULK_RET_OK && file_completed)
			*file_completed = 1;
	}
	
#ifdef ENABLE_METALINK
Exit:
	buffer->chunk = NULL;
	buffer->used_res = NULL;
#endif
	buffer->file_pt = NULL;
	buffer->id = NULL;
	buffer->url = NULL;
	buffer->uri = NULL;
	string_free(buffer->filename);

	return ret;
}

void print_buffers(void)
{
	int i;
	buffer_t *buffer;

	if (is_printf(MINFO)) {
		for (i = 0, buffer = buffer_array; i < option_values.max_sim_conns; i++, buffer++)
		{
#ifdef ENABLE_METALINK
			char *uri_str = uri2string(buffer->used_res->uri);
			MULK_INFO(("%d: %s, %s, %s", i, buffer->id ? _("PRESENT") : _("EMPTY"), 
				buffer->used_res ? uri_str : _("NULL"),
				buffer->chunk ? _("CHUNK") : _("NO CHUNK")));
			string_free(uri_str);
#else /* not ENABLE_METALINK */
			MULK_INFO(("%d: %s", i, buffer->id ? _("PRESENT") : _("EMPTY")));
#endif /* not ENABLE_METALINK */
			MULK_INFO(("\n"));
		} 
	}
}

void free_buffer_array(CURLM *curl_obj)
{
	int i;
	char *orig_url = NULL;
	buffer_t *buffer;
	CURLcode ret;

	if (!buffer_array)
		return;

	for (i = 0, buffer = buffer_array; i < option_values.max_sim_conns; i++, buffer++)
		if (buffer->file_pt)
			fclose(buffer->file_pt);

	for (i = 0, buffer = buffer_array; i < option_values.max_sim_conns; i++, buffer++) {
		if (!buffer->id)
			continue;

		ret = curl_easy_getinfo(buffer->id, CURLINFO_PRIVATE, &orig_url);
		curl_multi_remove_handle(curl_obj, buffer->id);
		if (ret == CURLE_OK)
			string_free(orig_url);
		curl_easy_cleanup(buffer->id);

#ifdef ENABLE_METALINK
		if (buffer->url->metalink_uri) {
			if (is_file_exist(buffer->filename)) {
				char *resume_filename = NULL;

				string_printf(&resume_filename, "%smetalink-mulkresume%05d",
					option_values.temp_directory, buffer->url->id);
				rename(buffer->filename, resume_filename);
				reset_metalink_file(buffer->url->metalink_uri, resume_filename);
				string_free(resume_filename);
			}
		}
		else 
#endif /* ENABLE_METALINK */
		{
			if (is_file_exist(buffer->filename))
				remove(buffer->filename);
		}
		string_free(buffer->filename);
		reset_url(buffer->url);
	}

	m_free(buffer_array);

	reset_url_list();
}
