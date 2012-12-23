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
#include "uri_parser.h"
#include "string_obj.h"


char *uri2string(UriUriA *uri)
{
	char *uri_str = NULL;
	int length = 0;

	if (!uri)
		return NULL;

	if (uriToStringCharsRequiredA(uri, &length) != URI_SUCCESS)
		return NULL;

	if ((uri_str = string_alloc(length)) == NULL)
		return NULL;

	length++;
	if (uriToStringA(uri_str, uri, length, NULL) != URI_SUCCESS) {
		string_free(uri_str);
		return NULL;
	}

	return uri_str;
}

char *uri2filename(UriUriA *uri)
{
	char *furi_str = uri2string(uri);
	char *newfilename = NULL;

	if (!furi_str)
		return NULL;

	string_replace_with_char(furi_str, "//", *DIR_SEPAR_STR);
	string_replace_with_char(furi_str, "/", *DIR_SEPAR_STR);

#ifdef _WIN32
	string_replace_with_char(furi_str, ":", '_');
#endif

	if (furi_str) {
		string_printf(&newfilename, "%s%s", option_values.file_output_directory, furi_str);

		if (newfilename[strlen(newfilename)-1] == *DIR_SEPAR_STR)
			string_cat(&newfilename, "index.html");
	}

	string_free(furi_str);

	return newfilename;
}

static UriUriA *uri_alloc_1(void)
{
	return m_calloc(1, sizeof(UriUriA));
}

void uri_free(UriUriA *uri)
{
	if (uri) {
		uriFreeUriMembersA(uri);
		m_free(uri);
	}
}

UriUriA *create_absolute_uri(const UriUriA *base_uri, const char *url)
{
	UriParserStateA state;
	UriUriA *abs_dest, rel_source;
	char *newurl;

	if (!url || !*url)
		return NULL;

	if ((abs_dest = uri_alloc_1()) == NULL)
		return NULL;

	if (base_uri) {
		state.uri = &rel_source;
		if (uriParseUriA(&state, url) != URI_SUCCESS) {
			uri_free(abs_dest); 
			uriFreeUriMembersA(&rel_source);
			return NULL;
		}

		if (uriAddBaseUriA(abs_dest, &rel_source, base_uri) != URI_SUCCESS) {
			uri_free(abs_dest);
			uriFreeUriMembersA(&rel_source);
			return NULL;
		}

		uriFreeUriMembersA(&rel_source);
	}
	else {
		state.uri = abs_dest;
		if (uriParseUriA(&state, url) != URI_SUCCESS) {
			uri_free(abs_dest); 
			return NULL;
		}
	}

	if (uriNormalizeSyntaxA(abs_dest) != URI_SUCCESS) {
		uri_free(abs_dest);
		return NULL;
	}

	/* http://www.example.com and http://www.example.com/ have to generate the same object */
	if (!base_uri && (!abs_dest->pathHead || !abs_dest->pathHead->text.first)
		&& !abs_dest->query.first) {
		newurl = string_new(url);
		string_cat(&newurl, "/");

		uriFreeUriMembersA(abs_dest);

		state.uri = abs_dest;
		if (uriParseUriA(&state, newurl) != URI_SUCCESS) {
			uri_free(abs_dest);
			string_free(newurl);
			return NULL;
		}

		if (uriNormalizeSyntaxA(abs_dest) != URI_SUCCESS) {
			uri_free(abs_dest);
			string_free(newurl);
			return NULL;
		}
		string_free(newurl);
	}

	return abs_dest;
}

UriUriA *filename2absolute_uri(const char *abs_filename)
{
	UriUriA *abs_uri, *file_uri = NULL;
	int length = 8 + 3 * strlen(abs_filename);
	char *abs_str_uri = string_alloc(length);

#ifdef _WIN32
	if (uriWindowsFilenameToUriStringA(abs_filename, abs_str_uri) != URI_SUCCESS)
#else
	if (uriUnixFilenameToUriStringA(abs_filename, abs_str_uri) != URI_SUCCESS)
#endif
	{
		string_free(abs_str_uri);
		return NULL;
	}

	file_uri = create_absolute_uri(NULL, "file:///");
	abs_uri = create_absolute_uri(file_uri, abs_str_uri);

	string_free(abs_str_uri);
	uri_free(file_uri);

	return abs_uri;
}

char *uri2absolute_filename(UriUriA* abs_uri)
{
	char *abs_filename = uri2string(abs_uri), *dst_filename = NULL;
	int length = strlen(abs_filename);

	dst_filename = string_alloc(length);

#ifdef _WIN32
	if (uriUriStringToWindowsFilenameA(abs_filename, dst_filename) != URI_SUCCESS)
#else
	if (uriUriStringToUnixFilenameA(abs_filename, dst_filename) != URI_SUCCESS)
#endif
		string_free(dst_filename);

	string_free(abs_filename);

	return dst_filename;
}

char *extract_relative_url(const char *src_filename, const char *base_filename)
{
	UriUriA *abs_src_uri, *abs_base_uri, dst;
	char *dst_filename = NULL;

	if (!src_filename || !base_filename)
		return NULL;

	abs_src_uri = filename2absolute_uri(src_filename);
	abs_base_uri = filename2absolute_uri(base_filename);

	if (!abs_src_uri || !abs_base_uri) {
		uri_free(abs_src_uri);
		uri_free(abs_base_uri);
		return NULL;
	}

	if (uriRemoveBaseUriA(&dst, abs_src_uri, abs_base_uri, URI_FALSE) != URI_SUCCESS)
		goto exit;

	dst_filename = uri2string(&dst);

exit:
	uri_free(abs_src_uri);
	uri_free(abs_base_uri);
	uriFreeUriMembersA(&dst);

	return dst_filename;
}

int	filter_uri(UriUriA **uri, int level)
{
	char *url;

	if (!option_values.exec_filter)
		return 0;

	if (!uri)
		return -1;

	if ((url = uri2string(*uri)) == NULL)
		return -1;

	uri_free(*uri);
	*uri = NULL;

	if (execute_filter(option_values.exec_filter, &url, level)) {
		string_free(url);
		return -1;
	}

	*uri = create_absolute_uri(NULL, url);
	string_free(url);

	if (!*uri)
		return -1;

	return 0;
}

int are_hosts_equal(UriUriA *first, UriUriA *second)
{
	if (first->hostData.ip4)
		return (second->hostData.ip4)
			&& !memcmp(first->hostData.ip4->data, second->hostData.ip4->data, 4);

	if (first->hostData.ip6) 
		return (second->hostData.ip6)
			&& !memcmp(first->hostData.ip6->data, second->hostData.ip6->data, 16);

	if (first->hostData.ipFuture.first)
		return (second->hostData.ipFuture.first)
			&& (first->hostData.ipFuture.afterLast - first->hostData.ipFuture.first) 
				== (second->hostData.ipFuture.afterLast - second->hostData.ipFuture.first)
			&& !strncmp(first->hostData.ipFuture.first, second->hostData.ipFuture.first,
				first->hostData.ipFuture.afterLast - first->hostData.ipFuture.first);

	if (first->hostText.first)
		return (second->hostText.first)
			&& (first->hostText.afterLast - first->hostText.first) 
				== (second->hostText.afterLast - second->hostText.first)
			&& !strncmp(first->hostText.first, second->hostText.first,
				first->hostText.afterLast - first->hostText.first);

	return !second->hostText.first;
}

char *get_host(UriUriA *uri)
{
	char *ret_str = NULL;
	int i, len = 0;

	if (uri->hostData.ip4)
		return *string_printf(&ret_str, "%d.%d.%d.%d", uri->hostData.ip4->data[0],
			uri->hostData.ip4->data[1], uri->hostData.ip4->data[2], uri->hostData.ip4->data[3]);
	
	if (uri->hostData.ip6) {
		ret_str = string_alloc(40);

		for (i = 0; i < 16; i++) {
			len += sprintf(ret_str + len, "%2.2x", uri->hostData.ip6->data[i]);
			if (i % 2 == 1 && i < 15)
				ret_str[len++] = ':';
		}
		return ret_str;
	}

	if (uri->hostData.ipFuture.first) 
		return string_nnew(uri->hostData.ipFuture.first,
			uri->hostData.ipFuture.afterLast - uri->hostData.ipFuture.first);

	if (uri->hostText.first) 
		return string_nnew(uri->hostText.first, uri->hostText.afterLast - uri->hostText.first);

	return NULL;
}

int is_uri_protocol(UriUriA *uri, const char *protocol)
{
	if (!uri || !uri->scheme.first || !uri->scheme.afterLast)
		return 0;

	if (((size_t) (uri->scheme.afterLast - uri->scheme.first)) == strlen(protocol)
		&& !string_ncasecmp(uri->scheme.first, protocol, uri->scheme.afterLast - uri->scheme.first))
		return 1;

	return 0;
}

int is_uri_http(UriUriA *uri)
{
	return is_uri_protocol(uri, HTTP_PROTOCOL) || is_uri_protocol(uri, HTTPS_PROTOCOL);
}

int is_uri_ftp(UriUriA *uri)
{
	return is_uri_protocol(uri, FTP_PROTOCOL);
}

int is_host_equal_domain(const char *host, const char *domain)
{
	if (!domain || !*domain || !host || !*host)
		return 0;

	return !string_casecmp(host, domain);
}

int is_host_in_domain(const char *host, const char *domain)
{
	int offset, len_dom, len_host;

	if (!domain || !*domain || !host || !*host)
		return 0;

	len_host = strlen(host);
	len_dom = strlen(domain);

	if ((offset = (len_host - len_dom)) < 0)
		return  0;

	/* to be sure to compare domains not substrings of them */
	if (*domain != DOMAIN_SEPARATOR && offset > 0 && *(host + offset - 1) != DOMAIN_SEPARATOR)
		return 0;

	return !string_casecmp(host + offset, domain);
}

int is_host_equal_domains(const char *host, char **domains)
{
	int i, ret = 0;

	if (!domains || !host)
		return 0;

	for (i = 0; domains[i]; i++)
		if (is_host_equal_domain(host, domains[i])) {
			ret = 1;
			break;
		}

	return ret;
}

int is_host_in_domains(const char *host, char **domains)
{
	int i, ret = 0;

	if (!domains || !host)
		return 0;

	for (i = 0; domains[i]; i++)
		if (is_host_in_domain(host, domains[i])) {
			ret = 1;
			break;
		}

	return ret;
}

