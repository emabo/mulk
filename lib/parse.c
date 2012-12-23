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


#include "parse.h"
#ifdef ENABLE_RECURSION
#if defined(HAVE_TIDY_H) || defined(_WIN32)
#include <tidy.h>
#include <buffio.h>
#elif defined(HAVE_TIDY_TIDY_H)
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#else
#error "Don't know where to look for libtidy headers"
#endif
#endif /* ENABLE_RECURSION */
#include "jpg_obj.h"
#include "buffer_array.h"
#include "file_obj.h"
#include "string_obj.h"
#ifdef ENABLE_METALINK
#include "chunk_list.h"
#endif


static size_t write_data_cb(void *ptr, size_t size, size_t nmemb, void *outstream)
{
	size_t nmemb_writed = 0;
	buffer_t *buffer = outstream;

	if (!buffer || !ptr)
		return 0;

	if (buffer->file_pt) {
#ifdef ENABLE_METALINK
		if (buffer->chunk) {
			off_t offset = buffer->chunk->start + buffer->chunk->pos, byte_writed;

			if ((buffer->chunk->pos + size * nmemb) > buffer->chunk->length)
				return 0;

			nmemb_writed = fwrite_offset(ptr, size, nmemb, offset, buffer->file_pt);
			fflush(buffer->file_pt);
			byte_writed = size * nmemb_writed;
			buffer->chunk->pos += byte_writed;

#ifdef ENABLE_CHECKSUM
		    if (buffer->chunk->checksum.cs_type != CS_NONE && byte_writed > 0)
				update_context(&buffer->chunk->checksum, ptr, byte_writed);
#endif /* ENABLE_CHECKSUM */
		}
		else
#endif /* ENABLE_METALINK */
		{
			nmemb_writed = fwrite(ptr, size, nmemb, buffer->file_pt);
			fflush(buffer->file_pt);
		}
	}

	return nmemb_writed * size;
}

mulk_type_return_t init_url(CURLM *cm)
{
	int i;
	url_list_t *url;
	UriUriA *uri;
#ifdef ENABLE_METALINK
	int header;
	chunk_t *chunk;
	metalink_resource_list_t *resource;
#endif
	char *str_url = NULL;
	CURL *eh;
	
	/* we need at least a free buffer */
	if (get_buffer(NULL) < 0)
		return MULK_RET_ERR;

#ifdef ENABLE_METALINK
	if ((url = search_next_url(&uri, &chunk, &resource, &header)) == NULL)
#else
	if ((url = search_next_url(&uri)) == NULL)
#endif
		return MULK_RET_ERR;

	if ((eh = curl_easy_init()) == NULL)
		return MULK_RET_ERR;

#ifdef ENABLE_METALINK
	i = open_buffer(eh, url, uri, chunk, resource, header);
#else
	i = open_buffer(eh, url, uri);
#endif

	str_url = uri2string(uri);

#ifdef ENABLE_METALINK
	if (header) {
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, NULL);
		curl_easy_setopt(eh, CURLOPT_NOBODY, 1L);
	}
	else
#endif /* ENABLE_METALINK */
	{
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_data_cb);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, &(buffer_array[i]));
		curl_easy_setopt(eh, CURLOPT_NOBODY, 0L);

#ifdef ENABLE_METALINK
		if (chunk) {
		    char *range = NULL;

			string_printf(&range, "%" PRIdMAX "-%" PRIdMAX, (intmax_t) chunk->start, 
				(intmax_t) (chunk->start + chunk->length - 1));

			curl_easy_setopt(eh, CURLOPT_RANGE, range);

			string_free(range);
		}
#endif /* ENABLE_METALINK */
	}

	curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
	curl_easy_setopt(eh, CURLOPT_USERAGENT, option_values.user_agent ? option_values.user_agent : "Mulk/" VERSION);
	curl_easy_setopt(eh, CURLOPT_URL, str_url);
	curl_easy_setopt(eh, CURLOPT_PRIVATE, str_url);
	curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(eh, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(eh, CURLOPT_CONNECTTIMEOUT, 30L);
	curl_easy_setopt(eh, CURLOPT_LOW_SPEED_LIMIT, 100L);
	curl_easy_setopt(eh, CURLOPT_LOW_SPEED_TIME, 30L);
	curl_easy_setopt(eh, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(eh, CURLOPT_SSL_VERIFYHOST, 0L);

	if (option_values.user || option_values.password) {
		char *usr_pwd = NULL;

		string_printf(&usr_pwd, "%s:%s", option_values.user ? option_values.user : "",
		   	option_values.password ? option_values.password : "");
		curl_easy_setopt(eh, CURLOPT_USERPWD, usr_pwd);

		string_free(usr_pwd);
	}

	if (option_values.proxy)
		curl_easy_setopt(eh, CURLOPT_PROXY, option_values.proxy);

	if (option_values.cookie)
		curl_easy_setopt(eh, CURLOPT_COOKIE, option_values.cookie);
	if (option_values.load_cookies)
		curl_easy_setopt(eh, CURLOPT_COOKIEFILE, option_values.load_cookies);
	if (option_values.save_cookies)
		curl_easy_setopt(eh, CURLOPT_COOKIEJAR, option_values.save_cookies);

	if (curl_multi_add_handle(cm, eh) != CURLM_OK)
		return MULK_RET_ERR;

	return MULK_RET_OK;
}

#ifdef ENABLE_RECURSION
static Bool TIDY_CALL filter_cb(TidyDoc tdoc, TidyReportLevel lvl, uint line, uint col, ctmbstr mssg)
{
	return no;
}

static void parse_html(TidyDoc tdoc, TidyNode tnod, const url_list_t *elem, int indent, FILE *outfile)
{
	TidyNode child;
	TidyAttr attr;
	TidyAttrId attr_id = TidyAttr_UNKNOWN;
	TidyNodeType node_type;
	TidyTagId node_id;
	ctmbstr name;
	char *url, *relative_url = NULL;
	int found = 0;
	int get_html_link = (!option_values.depth || elem->level < option_values.depth);
	int get_int_html_link = (!option_values.depth || elem->level < option_values.depth+1);
	int get_ext_depends = ((!option_values.depth || elem->level < option_values.depth+1)
		&& !option_values.no_html_dependencies);

	for (child = tidyGetChild(tnod); child; child = tidyGetNext(child)) {
		node_type = tidyNodeGetType(child);

		switch (node_type) {
			case TidyNode_Start:
			case TidyNode_StartEnd:
				node_id = tidyNodeGetId(child);
				if (get_html_link && (node_id == TidyTag_A || node_id == TidyTag_AREA || node_id == TidyTag_MAP)) {
					found = 1;
					attr_id = TidyAttr_HREF;
				}
				else if (get_int_html_link && (node_id == TidyTag_FRAME || node_id == TidyTag_IFRAME)) {
					found = 1;
					attr_id = TidyAttr_SRC; 
				}
				else if (get_ext_depends) {
					if (node_id == TidyTag_LINK) {
						found = 1;
						attr_id = TidyAttr_HREF;
					}
					else if (node_id == TidyTag_IMG || node_id == TidyTag_SCRIPT) {
						found = 1;
						attr_id = TidyAttr_SRC; 
					}
					else {
						found = 0;
						attr_id = TidyAttr_UNKNOWN;
					}
				}
				else {
					found = 0;
					attr_id = TidyAttr_UNKNOWN;
				}

				if (found && (attr = tidyAttrGetById(child, attr_id)) != NULL) {
					url = (char *) tidyAttrValue(attr);

					string_free(relative_url);
					if (url && *url)
						add_new_url_and_check(elem, url, outfile ? &relative_url : NULL);
				}

				if (outfile && (name = tidyNodeGetName(child)) != NULL) {
					fprintf(outfile, "%*.*s%s", indent, indent, "<", name);
					for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
						fprintf(outfile, " %s", tidyAttrName(attr));
						if (relative_url && (tidyAttrGetId(attr) == attr_id))
							fprintf(outfile, "=\"%s\"", relative_url);
						else if (tidyAttrValue(attr))
							fprintf(outfile, "=\"%s\"", tidyAttrValue(attr) ? tidyAttrValue(attr) : "");
						else
							fprintf(outfile, "=\"\"");
					}
					string_free(relative_url);

					if (node_type == TidyNode_StartEnd)
						fprintf(outfile, "/>\n");
					else {
						fprintf(outfile, ">\n");
						parse_html(tdoc, child, elem, indent + 1, outfile);
						fprintf(outfile, "%*.*s%s>\n", indent + 1, indent + 1, "</", name);
					}
				}
				else {
					string_free(relative_url);
					parse_html(tdoc, child, elem, indent + 1, outfile);
				}
				break;
			case TidyNode_End:
				if (outfile) {
					if ((name = tidyNodeGetName(child)) != NULL)
						fprintf(outfile, "%*.*s/%s>\n", indent, indent, "<", name);
				}
				break;
			case TidyNode_Text:
				if (outfile) {
					TidyBuffer buf;
					TidyTagId parent_node_id = tidyNodeGetId(tnod);

					tidyBufInit(&buf);
					if (parent_node_id == TidyTag_SCRIPT || parent_node_id == TidyTag_STYLE)
						tidyNodeGetValue(tdoc, child, &buf);
					else
						tidyNodeGetText(tdoc, child, &buf);
					if (buf.bp)
						fprintf(outfile, "%s", (char *)buf.bp);
					tidyBufFree(&buf);
				}
				break;
			case TidyNode_Comment:
				if (outfile) {
					TidyBuffer buf;

					tidyBufInit(&buf);
					tidyNodeGetValue(tdoc, child, &buf);
					if (buf.bp)
						fprintf(outfile, "<!--%s-->\n", (char *)buf.bp);
					tidyBufFree(&buf);
				}
				break;
			case TidyNode_CDATA:
				if (outfile) {
					TidyBuffer buf;

					tidyBufInit(&buf);
					tidyNodeGetValue(tdoc, child, &buf);
					if (buf.bp)
						fprintf(outfile, "<![CDATA[%s]]>\n", (char *)buf.bp);
					tidyBufFree(&buf);
				}
				break;
			case TidyNode_DocType:
				if (outfile) {
					int pub = 0;

					fprintf(outfile, "<!DOCTYPE %s", tidyNodeGetName(child));
					for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
						if (!pub) {
							fprintf(outfile, " %s", tidyAttrName(attr));
							if (!string_casecmp(tidyAttrName(attr), "PUBLIC"))
								pub = 1;
						}
						if (tidyAttrValue(attr))
							fprintf(outfile, " \"%s\"", tidyAttrValue(attr));
					}
					fprintf(outfile, ">\n");
				}
				break;
			default:
				if (outfile) {
					TidyBuffer buf;

					tidyBufInit(&buf);
					tidyNodeGetValue(tdoc, child, &buf);
					if (buf.bp)
						fprintf(outfile, "%s", (char *)buf.bp);
					tidyBufFree(&buf);
				}
				break;
		}
	}
}

void parse_urls(const char *filename, const url_list_t *elem)
{
	TidyDoc tdoc;
	int err;
	FILE *outfile = NULL;

	tdoc = tidyCreate();
	tidyOptSetBool(tdoc, TidyForceOutput, yes);
	tidyOptSetBool(tdoc, TidyMark, no);
	tidyOptSetBool(tdoc, TidyHideEndTags, yes);
	tidyOptSetBool(tdoc, TidyDropEmptyParas, no);
	tidyOptSetBool(tdoc, TidyJoinStyles, no);
	tidyOptSetBool(tdoc, TidyPreserveEntities, yes);
	tidyOptSetInt(tdoc, TidyMergeDivs, no);
	tidyOptSetInt(tdoc, TidyMergeSpans, no);
	tidyOptSetInt(tdoc, TidyWrapLen, 4096);
	tidyOptSetValue(tdoc, TidyCharEncoding, "utf8");
	tidySetReportFilter(tdoc, filter_cb);

	err = tidyParseFile(tdoc, filename);

	if (err >= 0) 
		err = tidyCleanAndRepair(tdoc);

	if (err >= 0) {
		outfile = option_values.save_relative_links && !option_values.disable_save_tree
			? fopen(filename, "w") : NULL;

		parse_html(tdoc, tidyGetRoot(tdoc), elem, 1, outfile);

		if (outfile)
			fclose(outfile);
	}

	tidyRelease(tdoc);
}
#endif /* ENABLE_RECURSION */
