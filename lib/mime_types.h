/*---------------------------------------------------------------------------
 * Copyright (C) 2008-2017 - Emanuele Bovisio
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

#ifndef _MIME_TYPES_H_
#define _MIME_TYPES_H_

#include "defines.h"

#define TYPE_GIF "image/gif"
#define TYPE_PNG "image/png"
#define TYPE_JPEG "image/jpeg"
#define TYPE_PJPEG "image/pjpeg"
#define TYPE_HTML "text/html"
#define TYPE_METALINK "application/metalink+xml"


int is_same_mimetype(const char *type1, const char *type2);
int is_jpeg_image(const char *type);
int is_gif_image(const char *type);
int is_png_image(const char *type);
int is_html_file(const char *type);
int is_metalink_file(const char *type);
int is_saved_mime_type(const char *type);
mulk_type_return_t extract_mime_type(const char *mime_type, char **type, char **subtype);

#endif /* not _MIME_TYPES_H_ */
