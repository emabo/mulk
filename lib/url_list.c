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

#include "url_list.h"
#include "buffer_array.h"
#include "string_obj.h"
#include "option_obj.h"
#include "file_obj.h"

#define REPORT_TEXT_EMPTY ""
#define REPORT_TEXT_NONE  "None"


url_list_t *top = NULL;
url_list_t *bottom = NULL;
url_list_t *download_ptr = NULL;
url_list_t *report_ptr = NULL;


int is_url_list_empty(void)
{
	return !top;
}

static url_list_t *create_new_element(void)
{
	url_list_t *elem;
	static int count = 1;

	elem = m_calloc(1, sizeof(url_list_t));
	elem->id = count++;
	elem->err_code = ERR_CODE_NOT_ASSIGNED;

	if (bottom)
		bottom->next = elem;
	bottom = elem;

	if (!top)
		top = elem;

	if (!download_ptr)
		download_ptr = elem;

	if (!report_ptr)
		report_ptr = elem;

	return elem;
}

static url_list_t *push_uri(UriUriA *uri, int level)
{
	url_list_t *elem;

	if (!uri)
		return NULL;

	if (filter_uri(&uri, level))
		return NULL;

	elem = create_new_element();
	elem->level = level;
	elem->uri = uri;
	elem->filename = uri2filename(uri);

	return elem;
}

static url_list_t *push_unique_uri(UriUriA *uri, int level)
{
	url_list_t *elem;
	
	if (!uri)
		return NULL;

	for (elem = top; elem; elem = elem->next)
		if (elem->uri && uriEqualsUriA(elem->uri, uri)) {
			uri_free(uri);
			return elem;
		}

	return push_uri(uri, level);
}

mulk_type_return_t mulk_add_new_url(const char *url)
{
	UriUriA *new_uri = NULL;
#ifdef ENABLE_RECURSION
	char *host = NULL;
#endif
	
	if ((new_uri = create_absolute_uri(NULL, url)) == NULL)
		goto Rejected;

#ifdef ENABLE_RECURSION
	if ((host = get_host(new_uri)) == NULL)
		goto Rejected; 
#endif

	if (!push_unique_uri(new_uri, 1))
		goto Rejected;

#ifdef ENABLE_RECURSION
	add_url_to_default_domains(host);

	string_free(host);
#endif

	return MULK_RET_OK;

Rejected:
#ifdef ENABLE_RECURSION
	string_free(host);
#endif
	uri_free(new_uri);

	return MULK_RET_URL_ERR;
}

#ifdef ENABLE_RECURSION
int add_new_url_and_check(const url_list_t *base_elem, const char *url, char **relative_url)
{
	UriUriA *uri = NULL;
	url_list_t *elem;

	if ((uri = create_absolute_uri(base_elem->uri, url)) == NULL)
		goto Rejected; 

	/* check protocol */
	if (!is_uri_http(uri) && (!option_values.follow_ftp || !is_uri_ftp(uri)))
		goto Rejected; 

	/* check domains */
	if (!is_host_compatible_with_domains(uri))
		goto Rejected; 

	if (!(elem = push_unique_uri(uri, base_elem->level + 1)))
		goto Rejected; 

	if (relative_url)
		*relative_url = extract_relative_url(elem->filename, base_elem->filename);

	return 0;

Rejected:
	uri_free(uri);
	return -1;
}
#endif /* ENABLE_RECURSION */

#ifdef ENABLE_METALINK
url_list_t *search_next_url(UriUriA **uri, chunk_t **chunk, metalink_resource_list_t **resource,
	int *header)
#else
url_list_t *search_next_url(UriUriA **uri)
#endif
{
	url_list_t *elem;
	int update_pointer = 1;

	for (elem = download_ptr; elem; elem = elem->next) {
		if (!elem->assigned && elem->err_code == ERR_CODE_NOT_ASSIGNED) {
			if (elem->uri) {
				if (is_uri_compatible(elem->uri, -1)) {
					elem->assigned = 1;
					*uri = elem->uri;
#ifdef ENABLE_METALINK
					*resource = NULL;
					*chunk = NULL;
					*header = 0;
#endif
					if (update_pointer)
						download_ptr = elem->next;
					return elem;
				}
				else 
					update_pointer = 0;
			}
#ifdef ENABLE_METALINK
			else if (elem->metalink_uri) {
				if (is_valid_metalink(elem->metalink_uri->file))
				{
					if (!elem->metalink_uri->chunk && elem->metalink_uri->size >= 0) {
						char *newfilename = NULL;

						if (create_chunks(elem->metalink_uri) != MULK_RET_OK) {
							elem->err_code = METALINK_RES_INVALID_METALINK;
							continue;
						}

#ifdef ENABLE_CHECKSUM
						/* load a resume file if present */
						if (init_chunks(elem->metalink_uri, &newfilename) == MULK_RET_OK) {
							elem->filename = newfilename;
							elem->err_code = METALINK_RES_OK;
							continue;
						}
#endif /* ENABLE_CHECKSUM */
						string_free(newfilename);
					}

					*uri = find_next_url(elem->metalink_uri, chunk, resource, header);

					if ((*chunk || *header) && *uri) {
						if (update_pointer)
							download_ptr = elem;
						return elem;
					}

					update_pointer = 0;
				}
				else 
					elem->err_code = METALINK_RES_INVALID_METALINK;
			}
#endif /* ENABLE_METALINK */
			else 
				elem->err_code = ERR_CODE_EMPTY_URL;
		}
	}

	if (update_pointer)
		download_ptr = NULL;
	*uri = NULL;
#ifdef ENABLE_METALINK
	*chunk = NULL;
	*resource = NULL;
	*header = 0;
#endif
	return NULL;
}

/* remember to free the url */
UriUriA *pop_url(void)
{
	url_list_t *elem;
	UriUriA *ret_uri;

	if (!top)
		return NULL;

	elem = top;
	ret_uri = top->uri;

	if (top == download_ptr)
		download_ptr = download_ptr->next;

	if (top == report_ptr)
		report_ptr = report_ptr->next;
	
	top = top->next;

	if (!top)
		bottom = NULL;

#ifdef ENABLE_METALINK
	free_metalink_file(elem->metalink_uri);
#endif
	string_free(elem->mimetype);
	string_free(elem->filename);
	string_free(elem->mimefilename);
	m_free(elem);

	return ret_uri;
}

void report_urls(void)
{
	url_list_t *elem;
	FILE *textfile = NULL, *csvfile = NULL;
	char *uri_str, *rep_uri, *rep_mime, *rep_file, *rep_mimefile;
	int update_pointer = 1;
	static int write_header = 1;
	char *text_filename = option_values.report_filename;
	char *csv_filename = option_values.report_csv_filename;

	if (text_filename && *text_filename)
		textfile = fopen(text_filename, "a");

	if (csv_filename && *csv_filename)
		csvfile = fopen(csv_filename, "a");

	if (!textfile && !csvfile)
		return;

	if (csvfile && write_header) {
		fprintf(csvfile, "ID,Depth,Error Code,HTTP Code,URL,Mime-Type,Filename,Mime-Type Filename\n");
		write_header = 0;
	}

	for (elem = report_ptr; elem; elem = elem->next) {
		if (elem->err_code == ERR_CODE_NOT_ASSIGNED) {
			update_pointer = 0;
			continue;
		}

		if (elem->reported)
			continue;

		uri_str = elem->uri ? uri2string(elem->uri) : NULL;

#ifdef ENABLE_METALINK
		rep_uri = elem->uri ? uri_str : (elem->metalink_uri ? elem->metalink_uri->file->name : REPORT_TEXT_NONE);
		rep_mime = elem->metalink_uri ? REPORT_TEXT_EMPTY : (elem->mimetype ? elem->mimetype : REPORT_TEXT_EMPTY);
#else
		rep_uri = elem->uri ? uri_str : REPORT_TEXT_NONE;
		rep_mime = elem->mimetype ? elem->mimetype : REPORT_TEXT_EMPTY;
#endif
		rep_file = elem->filename ? elem->filename : REPORT_TEXT_EMPTY;
		rep_mimefile = elem->mimefilename ? elem->mimefilename : REPORT_TEXT_EMPTY;

		if (textfile)
			fprintf(textfile, "ID:%d Depth:%d Error Code:%ld HTTP Code:%ld URL:\"%s\" Mime-Type:\"%s\" "
				"Filename:\"%s\" Mime-Type Filename:\"%s\"\n",
				elem->id, elem->level, elem->err_code, elem->http_code,
				rep_uri, rep_mime, rep_file, rep_mimefile);

		if (csvfile)
			fprintf(csvfile, "%d,%d,%ld,%ld,\"%s\",\"%s\",\"%s\",\"%s\"\n", 
				elem->id, elem->level, elem->err_code, elem->http_code,
				rep_uri, rep_mime, rep_file, rep_mimefile);

		elem->reported = 1;

		if (update_pointer)
			report_ptr = elem;

		string_free(uri_str);
	}

	if (textfile)
		fclose(textfile);

	if (csvfile)
		fclose(csvfile);
}

void free_urls(void)
{
	while (top) {
		UriUriA *uri = pop_url();
		if (uri)
			uri_free(uri);
	}
}

#ifdef ENABLE_METALINK
void push_metalink_uri(metalink_file_list_t *metalink_uri, int level)
{
	url_list_t *elem;

	if (!metalink_uri)
		return;

	elem = create_new_element();
	elem->level = level;
	elem->metalink_uri = metalink_uri;
	string_printf(&elem->filename, "%s%s", option_values.file_output_directory, metalink_uri->file->name);
}

void set_url_file_length(url_list_t *url, off_t size)
{
	if (url)
		set_metalink_file_length(url->metalink_uri, size);
}
#endif /* ENABLE_METALINK */

MULK_API mulk_type_return_t mulk_add_new_metalink_file(const char *metalink_filename)
{
#ifdef ENABLE_METALINK
	return add_new_metalink(metalink_filename, 1);
#else
	/* just to avoid compilation warning */
	return metalink_filename ? MULK_RET_OPTION_ERR : MULK_RET_OPTION_ERR;
#endif
}

void reset_url(url_list_t *url)
{
	if (!url)
		return;

	url->err_code = ERR_CODE_NOT_ASSIGNED;
	url->assigned = url->reported = url->tmp_file_created = 0;
	url->http_code = 0;
	string_free(url->mimetype);
	string_free(url->filename);
	string_free(url->mimefilename);
}

void reset_url_list(void)
{
	url_list_t *elem;
	int update_report_ptr = 1;

	for (elem = top; elem; elem = elem->next) {
		download_ptr = elem;
		if (update_report_ptr)
			report_ptr = elem;

		if (elem->err_code == ERR_CODE_NOT_ASSIGNED) 
			break;

		if (!elem->reported)
			update_report_ptr = 0;
	}
}
