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

#ifndef _OPTION_OBJ_H_
#define _OPTION_OBJ_H_

#include "defines.h"

#define OPT_OPTION_FILE "option-file"


typedef struct option_value_t {
	int quiet;
	int verbosity;
	char *user_agent;
	int max_sim_conns_per_host;
	int max_sim_conns;
	char *user;
	char *password;
	char *proxy;
	char *cookie;
	char *load_cookies;
	char *save_cookies;
	char *exec_filter;
#ifdef ENABLE_RECURSION
	int depth;
	int no_html_dependencies;
	int save_relative_links;
	int span_hosts;
	char *domains;
	char *exclude_domains;
	int follow_ftp;
#endif /* ENABLE_RECURSION */
	char *option_filename;
	char *url_filename;
#ifdef ENABLE_METALINK
	char *metalink_filename;
	char *metalink_list_filename;
	char *metalink_location;
	int metalink_print_locations;
	char *metalink_continent;
	int metalink_print_continents;
	char *metalink_os;
	char *metalink_language;
#ifdef ENABLE_CHECKSUM
	char *metalink_resume_file;
#endif /* ENABLE_CHECKSUM */
	int follow_metalink;
#endif /* ENABLE_METALINK */
	char *report_filename;
	char *report_csv_filename;
	int report_every_lines;
	int disable_save_tree;
	int save_gif_image;
	int save_png_image;
	int save_jpeg_image;
	char *save_mime_type;
	char *mime_output_directory;
	char *file_output_directory;
	char *temp_directory;
	int min_image_width;
	int max_image_width;
	int min_image_height;
	int max_image_height;
} option_value_t;

extern option_value_t option_values;
#ifdef ENABLE_CHECKSUM
extern int resume_file_used;
#endif

#ifdef ENABLE_RECURSION
void add_url_to_default_domains(const char *host);
int is_host_compatible_with_domains(UriUriA *uri);
#endif
int is_location_in_list(const char *loc);
void init_options(void);
void free_options(void);

#endif /* not _OPTION_OBJ_H_ */


