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

#include <ctype.h>
#include "mime_types.h"
#include "string_obj.h"

#define TYPE_DELIM ";: "
#define SUBTYPE_DELIM "/ "


int is_same_mimetype(const char *type1, const char *type2)
{
	return !string_ncasecmp(type1, type2, strlen(type2));
}

int is_gif_image(const char *type)
{
	return is_same_mimetype(type, TYPE_GIF);
}

int is_png_image(const char *type)
{
	return is_same_mimetype(type, TYPE_PNG);
}

int is_jpeg_image(const char *type)
{
	return is_same_mimetype(type, TYPE_JPEG) || is_same_mimetype(type, TYPE_PJPEG);
}

int is_html_file(const char *type)
{
	return is_same_mimetype(type, TYPE_HTML);
}

int is_metalink_file(const char *type)
{
	return is_same_mimetype(type, TYPE_METALINK);
}

int is_saved_mime_type(const char *type)
{
	return option_values.save_mime_type && is_same_mimetype(type, option_values.save_mime_type);
}

mulk_type_return_t extract_mime_type(const char *mime_type, char **type, char **subtype)
{
	char *token, *subtoken, *token_mime, *buf;

	if (!mime_type)
		return MULK_RET_ERR;

	if (type)
		*type = NULL;
	if (subtype)
		*subtype = NULL;

	buf = string_new(mime_type);
	if ((token = strtok(buf, TYPE_DELIM)) == NULL) {
		string_free(buf);
		return MULK_RET_ERR;
	}

	token_mime = string_new(token);
	string_lower(token_mime);

	string_free(buf);

	if ((subtoken = strtok(token_mime, SUBTYPE_DELIM)) == NULL) {
		string_free(token_mime);
		return MULK_RET_ERR;
	}

	if (type)
		*type = string_new(subtoken);

	if ((subtoken = strtok(NULL, SUBTYPE_DELIM)) == NULL) {
		string_free(token_mime);
		string_free(*type);
		return MULK_RET_ERR;
	}

	if (subtype)
		*subtype = string_new(subtoken);

	string_free(token_mime);

	return MULK_RET_OK;
}

