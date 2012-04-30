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

#include "metalink_list.h"
#include "string_obj.h"
#include "url_list.h"
#include "buffer_array.h"


metalink_list_t *metalink_top = NULL;
metalink_list_t *metalink_bottom = NULL;


/* metalink_resource_list_t */

static void push_metalink_resource(metalink_file_list_t *file, metalink_resource_t *resource)
{
	metalink_resource_list_t *elem;
	char *res_url = NULL;
	UriUriA* uri;

	if (!file || !resource)
		return;

	if (string_casecmp(resource->type, HTTP_PROTOCOL) && string_casecmp(resource->type, HTTPS_PROTOCOL)
		&& string_casecmp(resource->type, FTP_PROTOCOL)) {
		MULK_ERROR((_("ERROR: resource \"%s\" uses a not supported protocol (\"%s\")\n"), resource->url ? resource->url : "",
			resource->type ? resource->type : ""));
		return;
	}

	res_url = string_new(resource->url);
	string_trim(res_url);
	uri = create_absolute_uri(NULL, res_url);

	if (!is_uri_protocol(uri, resource->type)) {
		MULK_ERROR((_("ERROR: resource \"%s\" has an inconsistent protocol (\"%s\")\n"), resource->url ? resource->url : "",
			resource->type ? resource->type : ""));
		uri_free(uri);
		string_free(res_url);
		return;
	}

	elem = m_calloc(1, sizeof(metalink_resource_list_t));
	elem->resource = resource;
	elem->uri = uri;
	elem->prev = file->usable_res_bottom;

	if (file->usable_res_bottom)
		file->usable_res_bottom->next = elem;
	file->usable_res_bottom = elem;

	if (!file->usable_res_top)
		file->usable_res_top = elem;

	string_free(res_url);
}

void remove_metalink_resource(metalink_file_list_t *file, metalink_resource_list_t *resource)
{
	metalink_resource_list_t *prev, *next;

	if (!file || !resource || !file->usable_res_top)
		return;

	if (resource->assigned) {
		resource->error = 1;
		return;
	}

	prev = resource->prev;
	next = resource->next;

	if (file->usable_res_top == resource)
		file->usable_res_top = next;
	if (file->usable_res_bottom == resource)
		file->usable_res_bottom = prev;

	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;

	if (resource->uri)
		uri_free(resource->uri);
	m_free(resource);
}

static int is_valid_resource(metalink_resource_t *resource)
{
	return ((!string_casecmp(resource->type, HTTP_PROTOCOL)
		|| !string_casecmp(resource->type, HTTPS_PROTOCOL)
		|| !string_casecmp(resource->type, FTP_PROTOCOL))
	   	&& is_location_in_list(resource->location));
}

int is_resource_available(metalink_file_list_t *file, int header)
{
	metalink_resource_list_t *elem;

	if (!file)
		return 0;

	for (elem = file->usable_res_top; elem; elem = elem->next) {
		if (!elem->error && is_valid_resource(elem->resource)
				/* only HTTP or HTTPS to read filesize information */
				&& (!header || (header && string_casecmp(elem->resource->type, FTP_PROTOCOL)))) 
			return 1;
	} 

	return 0;
}

static metalink_resource_list_t *find_free_resource(metalink_file_list_t *file, int header)
{
	metalink_resource_list_t *elem = NULL, *max_elem = NULL;
	int max_preference = -10;

	if (!file)
		return NULL;

	if (file->file->maxconnections > 0 && count_number_of_chunks(file) >= file->file->maxconnections)
		return NULL;

	for (elem = file->usable_res_top; elem; elem = elem->next) {
		if (!elem->error 
				&& elem->resource->preference > max_preference 
				&& is_valid_resource(elem->resource)
				&& is_uri_compatible(elem->uri, elem->resource->maxconnections)
				/* only HTTP or HTTPS to read filesize information */
				&& (!header || (header && string_casecmp(elem->resource->type, FTP_PROTOCOL)))) {
			max_preference = elem->resource->preference;
			max_elem = elem;
		}
	} 

	if (max_elem)
		max_elem->assigned++;

	return max_elem;
}

void free_resources(metalink_file_list_t *file)
{
	metalink_resource_list_t *tmp, *elem;

	if (!file)
		return;

	elem = file->usable_res_top;

	while (elem) {
		tmp = elem;
		elem = elem->next;
		if (tmp->uri)
			uri_free(tmp->uri);
		m_free(tmp);
	}
	file->usable_res_top = file->usable_res_bottom = NULL;
}

static void reset_resources(metalink_file_list_t *file)
{
	metalink_resource_list_t *elem;

	if (!file)
		return;

	for (elem = file->usable_res_top; elem; elem = elem->next)
		elem->assigned = elem->error = 0;
}

/* metalink_file_list_t */

metalink_file_list_t *create_metalink_file(metalink_file_t *file)
{
	metalink_file_list_t *elem;

	if (!file)
		return NULL;

	elem = m_calloc(1, sizeof(metalink_file_list_t));
	elem->file = file;
	elem->size = file->size ? file->size : -1L;

	return elem;
}

UriUriA *find_next_url(metalink_file_list_t *file, chunk_t **chunk, metalink_resource_list_t **resource, 
	int *header)
{
	*chunk = find_free_chunk(file);

	if (file->size >= 0) {
		*header = 0;
		if (!*chunk || !is_resource_available(file, 0)) {
			/* all chunks have an url assigned or there aren't anymore urls */
			*resource = NULL;
			*chunk = NULL;
			return NULL;
		}
	}
	else if (file->header) {
		/* already downloading the header */
		*resource = NULL;
		*chunk = NULL;
		*header = 0;
		return NULL;
	}
	else {
		*header = 1;
		file->header = 1;
	}

	if (!(*resource = find_free_resource(file, *header))) {
		file->header = 0;
		return NULL;
	}

	if (*chunk) {
		(*chunk)->used_res = *resource;
		(*chunk)->pos = 0;
	}

	return (*resource)->uri;
}

void set_metalink_file_length(metalink_file_list_t *file, off_t size)
{
	if (file)
		file->size = size;
}

void free_metalink_file(metalink_file_list_t *file)
{
	if (!file)
		return;

	free_chunks(file);
	free_resources(file);
	string_free(file->resume_filename);

	m_free(file);
}

void reset_metalink_file(metalink_file_list_t *file, const char *resume_filename)
{
	if (!file)
		return;

	free_chunks(file);
	reset_resources(file);

	file->chunk_number = 0;
	file->header = 0;
	string_free(file->resume_filename);
	file->resume_filename = string_new(resume_filename);
}

/* metalink_list_t */

void push_metalink(metalink_t *metalink)
{
	metalink_list_t *elem;

	if (!metalink)
		return;

	elem = m_calloc(1, sizeof(metalink_list_t));
	elem->metalink = metalink;

	if (metalink_bottom)
		metalink_bottom->next = elem;
	metalink_bottom = elem;

	if (!metalink_top)
		metalink_top = elem;
}

void free_metalinks(void)
{
	metalink_list_t *tmp;
	while (metalink_top) {
		tmp = metalink_top;
		metalink_delete(metalink_top->metalink);
		metalink_top = metalink_top->next;
		m_free(tmp);
	}
}

int is_valid_metalink(metalink_file_t* file)
{
	/* resources not present */
	if (!file || !file->resources || !file->resources[0])
		return 0;

	/* different OS */
	if (option_values.metalink_os && *(option_values.metalink_os)
		&& !string_casestr(file->os, option_values.metalink_os))
		return 0;

	/* different language */
	if (option_values.metalink_language && *(option_values.metalink_language)
		&& string_casecmp(file->language, option_values.metalink_language))
		return 0;

	return 1;
}

mulk_type_return_t add_new_metalink(const char *filename, int level)
{
	metalink_error_t err;
	metalink_t* metalink;
	metalink_file_t** files;
	metalink_resource_t** resources;
	metalink_file_list_t* new_file;

	err = metalink_parse_file(filename, &metalink);

	if (err || !metalink || !metalink->files) {
		MULK_ERROR((_("ERROR: fail parsing Metalink file, error code=%d\n"), err));
		return MULK_RET_FILE_ERR;
	}

	push_metalink(metalink);

	for (files = metalink->files; *files; files++) {
		MULK_DEBUG((_("name = %s\n"), (*files)->name ? (*files)->name : "(null)"));

		if (!(new_file = create_metalink_file(*files)))
			continue;

		for (resources = (*files)->resources; *resources; resources++) {
			MULK_DEBUG((_("resource = %s\n"), (*resources)->url ? (*resources)->url : "(null)"));
			push_metalink_resource(new_file, *resources);
		}

		if (new_file->usable_res_top)
			push_metalink_uri(new_file, level);
	}

	return MULK_RET_OK;
}
