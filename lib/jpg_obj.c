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

#include "jpg_obj.h"

#define JPEG_DIM_SIZE  5

#define JPEG_SOF_START 0xC0
#define JPEG_DHT       0xC4
#define JPEG_JPG       0xC8
#define JPEG_DAC       0xCC
#define JPEG_SOF_END   0xCF
#define JPEG_SOI       0xD8
#define JPEG_EOI       0xD9
#define JPEG_SOS       0xDA
#define JPEG_APP0      0xE0
#define JPEG_FLAG      0xFF

#define MAKE_WORD(h, l) (((h) << 8) | (l))

static int is_start_of_frame(unsigned char value)
{
	return (value >= JPEG_SOF_START && value <= JPEG_SOF_END
		&& value != JPEG_DHT && value != JPEG_JPG && value != JPEG_DAC);
}

static int read_jpeg_mark(FILE * file, unsigned char *mark, int *dim)
{
	unsigned char buffer[2];

	do {
		if (!fread(buffer, 1, 1, file))
			return 0;
	}
	while (*buffer != 0xFF);

	while (*buffer == 0xFF)
		if (!fread(buffer, 1, 1, file))
			return 0;

	*mark = *buffer;

	if (fread(buffer, 1, 2, file) != 2)
		return 0;

	*dim = MAKE_WORD(buffer[0], buffer[1]);

	return 1;
}

static mulk_type_return_t read_jpeg_dim(const char *filename, int *width, int *height)
{
	unsigned char mark, buffer[JPEG_DIM_SIZE];
	int size;
	mulk_type_return_t ret = MULK_RET_ERR;
	int first_time = 1;
	FILE *file = NULL;

	if ((file = fopen(filename, "rb")) == NULL)
		return MULK_RET_FILE_ERR;

	if (fread(buffer, 1, 2, file) != 2)
		goto Exit;

	if (buffer[0] != JPEG_FLAG || buffer[1] != JPEG_SOI)
		goto Exit;

	while (read_jpeg_mark(file, &mark, &size))
	{
		if (mark == JPEG_SOS)
			break;

		else if (first_time)
		{
			if (mark != JPEG_APP0)
				goto Exit;

			if (fread(buffer, 1, JPEG_DIM_SIZE, file) != JPEG_DIM_SIZE)
				goto Exit;

			if (strcmp((char *)buffer, "JFIF"))
				goto Exit;

			if (mulk_fseek(file, size - 2 - JPEG_DIM_SIZE, SEEK_CUR))
				goto Exit;
		}
		else if (is_start_of_frame(mark))
		{
			if (fread(buffer, 1, JPEG_DIM_SIZE, file) != JPEG_DIM_SIZE)
				goto Exit;

			*height = MAKE_WORD(buffer[1], buffer[2]);
			*width = MAKE_WORD(buffer[3], buffer[4]);

			ret = MULK_RET_OK;
			goto Exit;

		}
		else
		{
			if (mulk_fseek(file, size - 2, SEEK_CUR))
				goto Exit;
		}
		first_time = 0;
	}

Exit:
	fclose(file);
	return ret;
}

int is_valid_jpeg_image(const char *filename)
{
	int width = 0, height = 0, valid;

	if (!option_values.save_jpeg_image)
		return 0;

	valid = (read_jpeg_dim(filename, &width, &height) == MULK_RET_OK);

	valid = valid && (width >= option_values.min_image_width) 
		&& (!option_values.max_image_width || width <= option_values.max_image_width) 
		&& (height >= option_values.min_image_height) 
		&& (!option_values.max_image_height || height <= option_values.max_image_height);

	MULK_INFO((_("Image JPG %s: %s, width = %d, height = %d\n"), valid ? "OK" : "FILTERED",
		filename, width, height));

	return valid;
}
