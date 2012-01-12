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

#include "gif_obj.h"

#define GIF_HEADER_SIZE 6
#define GIF_DIM_SIZE 4

#define GIF_HEADER_VAL1 "GIF87a"
#define GIF_HEADER_VAL2 "GIF89a"

#define MAKE_WORD(h, l) (((h) << 8) | (l))

static mulk_type_return_t read_gif_dim(const char *filename, int *width, int *height)
{
	unsigned char buffer[GIF_HEADER_SIZE + GIF_DIM_SIZE], *dim;
	mulk_type_return_t ret = MULK_RET_ERR;
	size_t bytestoread;
	FILE *file = NULL;

	if ((file = fopen(filename, "rb")) == NULL)
		return MULK_RET_FILE_ERR;

	bytestoread = GIF_HEADER_SIZE + GIF_DIM_SIZE;
	if (fread(buffer, 1, bytestoread, file) != bytestoread)
		goto Exit;

	if (memcmp(buffer, GIF_HEADER_VAL1, GIF_HEADER_SIZE)
		&& memcmp(buffer, GIF_HEADER_VAL2, GIF_HEADER_SIZE))
		goto Exit;

	dim = buffer + GIF_HEADER_SIZE;
	*width = MAKE_WORD(dim[1], dim[0]);
	*height = MAKE_WORD(dim[3], dim[2]);

	ret = MULK_RET_OK;

Exit:
	fclose(file);
	return ret;
}

int is_valid_gif_image(const char *filename)
{
	int width = 0, height = 0, valid;

	if (!option_values.save_gif_image)
		return 0;

	valid = (read_gif_dim(filename, &width, &height) == MULK_RET_OK);

	valid = valid && (width >= option_values.min_image_width) 
		&& (!option_values.max_image_width || width <= option_values.max_image_width) 
		&& (height >= option_values.min_image_height) 
		&& (!option_values.max_image_height || height <= option_values.max_image_height);

	MULK_INFO((_("Image GIF %s: %s, width = %d, height = %d\n"), valid ? "OK" : "FILTERED",
		filename, width, height));

	return valid;
}
