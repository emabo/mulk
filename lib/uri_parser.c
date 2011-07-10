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
		string_free(&uri_str);
		return NULL;
	}

	return uri_str;
}

char *uri2filename(UriUriA *uri)
{
	char *uri_str = uri2string(uri);

	if (!uri_str)
		return NULL;

	string_replace_with_char(uri_str, "//", *DIR_SEPAR_STR);
	string_replace_with_char(uri_str, "/", *DIR_SEPAR_STR);
	
#ifdef _WIN32
	string_replace_with_char(uri_str, ":", '_');
#endif

	return uri_str;
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

UriUriA *create_absolute_uri(const char *base_url, const char *url)
{
	UriParserStateA state;
	UriUriA *abs_dest, abs_base, rel_source;
	char *newurl;

	if (!url || !*url)
		return NULL;

	if ((abs_dest = uri_alloc_1()) == NULL)
		return NULL;

	if (base_url) {
		state.uri = &rel_source;
		if (uriParseUriA(&state, url) != URI_SUCCESS) {
			m_free(abs_dest); 
			uriFreeUriMembersA(&rel_source);
			return NULL;
		}

		state.uri = &abs_base;
		if (uriParseUriA(&state, base_url) != URI_SUCCESS) {
			m_free(abs_dest); 
			uriFreeUriMembersA(&abs_base);
			uriFreeUriMembersA(&rel_source);
			return NULL;
		}

		if (uriAddBaseUriA(abs_dest, &rel_source, &abs_base) != URI_SUCCESS) {
			uri_free(abs_dest);
			uriFreeUriMembersA(&abs_base);
			uriFreeUriMembersA(&rel_source);
			return NULL;
		}

		uriFreeUriMembersA(&abs_base);
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
	if (!base_url && (!abs_dest->pathHead || !abs_dest->pathHead->text.first)
		&& !abs_dest->query.first) {
		newurl = string_new(url);
		string_cat(&newurl, "/");

		uriFreeUriMembersA(abs_dest);

		state.uri = abs_dest;
		if (uriParseUriA(&state, newurl) != URI_SUCCESS) {
			uri_free(abs_dest);
			string_free(&newurl);
			return NULL;
		}

		if (uriNormalizeSyntaxA(abs_dest) != URI_SUCCESS) {
			uri_free(abs_dest);
			string_free(&newurl);
			return NULL;
		}
		string_free(&newurl);
	}

	return abs_dest;
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

int is_host_equal_domains(UriUriA *uri, char **domains)
{
	int i, ret = 0;
	char *host;

	if (!domains || !uri)
		return 0;

	if ((host = get_host(uri)) == NULL)
		return 0;

	for (i = 0; domains[i]; i++)
		if (is_host_equal_domain(host, domains[i])) {
			ret = 1;
			break;
		}

	string_free(&host);
	return ret;
}

int is_host_in_domains(UriUriA *uri, char **domains)
{
	int i, ret = 0;
	char *host;

	if (!domains || !uri)
		return 0;

	if ((host = get_host(uri)) == NULL)
		return 0;

	for (i = 0; domains[i]; i++)
		if (is_host_in_domain(host, domains[i])) {
			ret = 1;
			break;
		}

	string_free(&host);
	return ret;
}
