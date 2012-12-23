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

#include "option_obj.h"
#ifdef _WIN32
#include "../win32/getopt.h"
#else
#include <getopt.h>
#endif
#include "file_obj.h"
#include "url_list.h"
#include "string_obj.h"
#ifdef ENABLE_METALINK
#include "metalink_list.h"
#include "country_codes.h"

#define LOC_LENGTH 2
#define LOC_OPTION_DELIM ",;.:- "
#endif

#define NUM_OPTIONS ((int) (sizeof(options) / sizeof(option_t)))

#define DEFAULT_MIME_OUTPUT_DIRECTORY "data"
#define DEFAULT_FILE_OUTPUT_DIRECTORY "."
#define DEFAULT_TEMP_DIRECTORY ".tmp-mulk"


typedef mulk_type_return_t (*set_option_value_t)(void);

typedef enum type_option_t {
	OPTION_BOOL,
	OPTION_INT,
	OPTION_STRING,
	OPTION_HELP,
	OPTION_VERSION
} type_option_t;

typedef struct option_t {
	char *long_opt;
	char short_opt;
	char *description;
	void *value;
	type_option_t type;
	int min_val;
	int max_val;
	char *error;
	char *title;
	set_option_value_t set_option_value;
} option_t;

#ifdef ENABLE_RECURSION
static char **default_domains = NULL;
static char **accepted_domains = NULL;
static char **rejected_domains = NULL;
#endif /* ENABLE_RECURSION */
#ifdef ENABLE_METALINK
static char *location_countries = NULL;
static char *location_continents = NULL;
#ifdef ENABLE_CHECKSUM
int resume_file_used = 0;
#endif
#endif /* ENABLE_METALINK */

static mulk_type_return_t set_option_option_filename(void);
static mulk_type_return_t set_option_url_filename(void);
static mulk_type_return_t set_option_report_filename(void);
static mulk_type_return_t set_option_report_csv_filename(void);
static mulk_type_return_t set_option_mime_output_directory(void);
static mulk_type_return_t set_option_file_output_directory(void);
static mulk_type_return_t set_option_temp_directory(void);
#ifdef ENABLE_RECURSION
static mulk_type_return_t set_option_domains(void);
static mulk_type_return_t set_option_exclude_domains(void);
#endif
#ifdef ENABLE_METALINK
static mulk_type_return_t set_option_metalink_filename(void);
static mulk_type_return_t set_option_metalink_list_filename(void);
static mulk_type_return_t set_option_metalink_location(void);
static mulk_type_return_t set_option_metalink_continent(void);
#ifdef ENABLE_CHECKSUM
static mulk_type_return_t set_option_metalink_resume_file(void);
#endif
#endif /* ENABLE_METALINK */

/* reset all struct */
option_value_t option_values = {0};

option_t options[] = {
	{"version",                  0, gettext_noop("display version number and exit"),
		NULL,                                     OPTION_VERSION, 0, 0, NULL,
		gettext_noop("General options"), NULL},
	{"help",                   'h', gettext_noop("display this help and exit"),
		NULL,                                     OPTION_HELP, 0, 0, NULL, NULL, NULL},
	{"quiet",                  'q', gettext_noop("quiet (no output)"),
		&option_values.quiet,                     OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"verbose",                'v', gettext_noop("be verbose"),
		&option_values.verbosity,                 OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"user-agent",             'U', gettext_noop("identify as different agent instead of Mulk/VERSION"),
		&option_values.user_agent,                OPTION_STRING, 0, 0, NULL,
		gettext_noop("Download options"), NULL},
	{"max-sim-conns-per-host", 'p', gettext_noop("maximum simultaneous connections per host"), 
		&option_values.max_sim_conns_per_host,    OPTION_INT, 1, 5,
		gettext_noop("wrong value for maximum simultaneous connections per host"), NULL, NULL},
	{"max-sim-conns",          'm', gettext_noop("maximum simultaneous connections"),
		&option_values.max_sim_conns,             OPTION_INT, 1, 50,
		gettext_noop("wrong value for maximum simultaneous connections"), NULL, NULL},
	{"user",                     0, gettext_noop("username to use for the connection"),
		&option_values.user,                      OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"password",                 0, gettext_noop("password to use for the connection"),
		&option_values.password,                  OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"proxy",                    0, gettext_noop("<host[:port]> use HTTP proxy on given host and port"),
		&option_values.proxy,                     OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"cookie",                 'b', gettext_noop("cookie string for HTTP session"),
		&option_values.cookie,                    OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"load-cookies",             0, gettext_noop("load cookies from file for HTTP session"),
		&option_values.load_cookies,              OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"save-cookies",             0, gettext_noop("save cookies to file after HTTP session"),
		&option_values.save_cookies,              OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"exec-filter",              0, gettext_noop("execute an external program to filter or modify URLs to download"),
		&option_values.exec_filter,               OPTION_STRING, 0, 0, NULL, NULL, NULL},
#ifdef ENABLE_RECURSION
	{"depth",                  'd', gettext_noop("maximum recursion depth (0 for infinite)"),
		&option_values.depth,                     OPTION_INT, 0, INT_MAX, gettext_noop("wrong depth values"),
		gettext_noop("Recursive download options"), NULL},
	{"no-html-dependencies",     0, gettext_noop("don't get all images, links, etc. needed to display HTML page"),
		&option_values.no_html_dependencies,      OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"save-relative-links",    'k', gettext_noop("make links relative in downloaded HTML pages"),
		&option_values.save_relative_links,       OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"span-hosts",             'H', gettext_noop("go to foreign hosts"),
		&option_values.span_hosts,                OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"domains",                'D', gettext_noop("comma-separated list of accepted domains"),
		&option_values.domains,                   OPTION_STRING, 0, 0, NULL, NULL, set_option_domains},
	{"exclude-domains",          0, gettext_noop("comma-separated list of rejected domains"),
		&option_values.exclude_domains,           OPTION_STRING, 0, 0, NULL, NULL, set_option_exclude_domains},
	{"follow-ftp",               0, gettext_noop("follow FTP links from HTML documents"),
		&option_values.follow_ftp,                OPTION_BOOL, 0, 0, NULL, NULL, NULL},
#endif /* ENABLE_RECURSION */
	{OPT_OPTION_FILE,          't', gettext_noop("text file with list of options"),
		&option_values.option_filename,           OPTION_STRING, 0, 0, NULL,
		gettext_noop("Input options"), set_option_option_filename},
	{"url-file",               'f', gettext_noop("text file with list of URLs to download"),
		&option_values.url_filename,              OPTION_STRING, 0, 0, NULL, NULL, set_option_url_filename},
#ifdef ENABLE_METALINK
	{"metalink-file",          'l', gettext_noop("Metalink filename"),
		&option_values.metalink_filename,         OPTION_STRING, 0, 0, NULL,
		gettext_noop("Metalink options"), set_option_metalink_filename},
	{"metalink-list-file",       0, gettext_noop("text file with list of Metalink files to download"),
		&option_values.metalink_list_filename,    OPTION_STRING, 0, 0, NULL, NULL, set_option_metalink_list_filename},
	{"metalink-location",        0, gettext_noop("comma-separated list of accepted countries"),
		&option_values.metalink_location,         OPTION_STRING, 0, 0, NULL, NULL, set_option_metalink_location},
	{"metalink-print-locations", 0, gettext_noop("print full list of valid countries"),
		&option_values.metalink_print_locations,  OPTION_HELP, 0, 0, NULL, NULL, NULL},
	{"metalink-continent",       0, gettext_noop("comma-separated list of accepted continents"),
		&option_values.metalink_continent,        OPTION_STRING, 0, 0, NULL, NULL, set_option_metalink_continent},
	{"metalink-print-continents",0, gettext_noop("print full list of valid continents"),
		&option_values.metalink_print_continents, OPTION_HELP, 0, 0, NULL, NULL, NULL},
	{"metalink-os",              0, gettext_noop("operating system to be considered in Metalink files"),
		&option_values.metalink_os,               OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"metalink-language",        0, gettext_noop("language to be considered in Metalink files"),
		&option_values.metalink_language,         OPTION_STRING, 0, 0, NULL, NULL, NULL},
#ifdef ENABLE_CHECKSUM
	{"metalink-resume-file",     0, gettext_noop("filename to resume"),
		&option_values.metalink_resume_file,      OPTION_STRING, 0, 0, NULL, NULL, set_option_metalink_resume_file},
#endif /* ENABLE_CHECKSUM */
	{"follow-metalink",          0, gettext_noop("follow Metalink files from HTML documents"),
		&option_values.follow_metalink,           OPTION_BOOL, 0, 0, NULL, NULL, NULL},
#endif /* ENABLE_METALINK */
	{"report-file",            'r', gettext_noop("report filename"),
		&option_values.report_filename,           OPTION_STRING, 0, 0, NULL,
		gettext_noop("Reporting options"), set_option_report_filename},
	{"report-csv-file",        'c', gettext_noop("report CSV filename"),
		&option_values.report_csv_filename,       OPTION_STRING, 0, 0, NULL, NULL, set_option_report_csv_filename},
	{"report-every-lines",       0, gettext_noop("write report to file every n lines"),
		&option_values.report_every_lines,        OPTION_INT, 0, INT_MAX,
		gettext_noop("wrong maximum number of lines for writing report"), NULL, NULL},
	{"disable-site-save",      'x', gettext_noop("don't save whole site tree to disk (enabled by default)"),
		&option_values.disable_save_tree,         OPTION_BOOL, 0, 0, NULL,
		gettext_noop("Saving options"), NULL},
	{"save-gif-image",         'g', gettext_noop("save GIF images to mime output directory"),
		&option_values.save_gif_image,            OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"save-png-image",         'n', gettext_noop("save PNG images to mime output directory"),
		&option_values.save_png_image,            OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"save-jpeg-image",        'j', gettext_noop("save JPEG images to mime output directory"),
		&option_values.save_jpeg_image,           OPTION_BOOL, 0, 0, NULL, NULL, NULL},
	{"save-mime-type",           0, gettext_noop("save URLs with specific mime-type to mime output directory"),
		&option_values.save_mime_type,            OPTION_STRING, 0, 0, NULL, NULL, NULL},
	{"mime-output-dir",          0, gettext_noop("mime output directory"),
		&option_values.mime_output_directory,     OPTION_STRING, 0, 0, NULL, NULL, set_option_mime_output_directory},
	{"file-output-dir",          0, gettext_noop("file output directory"),
		&option_values.file_output_directory,     OPTION_STRING, 0, 0, NULL, NULL, set_option_file_output_directory},
	{"temp-dir",                 0, gettext_noop("temporary directory"),
		&option_values.temp_directory,            OPTION_STRING, 0, 0, NULL, NULL, set_option_temp_directory},
	{"min-image-width",          0, gettext_noop("minimum image width"),
		&option_values.min_image_width,           OPTION_INT, 0, INT_MAX,
		gettext_noop("wrong minimum image width"),
		gettext_noop("Image options (active only with --save-gif-image, --save-png-image or --save-jpeg-image)"), NULL},
	{"max-image-width",          0, gettext_noop("maximum image width"),
		&option_values.max_image_width,           OPTION_INT, 0, INT_MAX,
		gettext_noop("wrong maximum image width"), NULL, NULL},
	{"min-image-height",         0, gettext_noop("minimum image height"),
		&option_values.min_image_height,          OPTION_INT, 0, INT_MAX,
		gettext_noop("wrong minimum image height"), NULL, NULL},
	{"max-image-height",         0, gettext_noop("maximum image height"),
		&option_values.max_image_height,          OPTION_INT, 0, INT_MAX,
		gettext_noop("wrong maximum image height"), NULL, NULL},
};

#ifdef ENABLE_RECURSION
/* domains options */

static int is_domain_empty(char **domains)
{
	return !domains || !*domains || !**domains;
}

static int count_domains(char **domains)
{
	int i;

	if (!domains || !*domains || !**domains)
		return 0;

	for (i = 0; domains[i]; i++);

	return i;
}

void add_url_to_default_domains(const char *host)
{
	int i, dom_num;
	char **new_default_domains = NULL;

	if (!host)
		return;

	/* yet present */
	if (is_host_equal_domains(host, default_domains))
		return;

	dom_num = count_domains(default_domains);

	new_default_domains = m_calloc(dom_num + 2, sizeof(char*));

	if (default_domains) {
		for (i = 0; default_domains[i]; i++)
			new_default_domains[i] = default_domains[i];

		m_free(default_domains);
	}

	new_default_domains[dom_num] = string_new(host);

	default_domains = new_default_domains;
}

static mulk_type_return_t parse_domains(const char *dom_option, char ***domains)
{
	char *buf = NULL;
	char *s, *res;
	int dom_num, i;

	if (!domains || !dom_option || !*dom_option)
		return MULK_RET_ERR;

	if (*domains)
		m_free(*domains);

	buf = string_new(dom_option);
	s = buf;
	dom_num = 0;
	while (strtok(s, DOMAIN_OPTION_DELIM)) {
		dom_num++;
		s = NULL;
	}
	string_free(buf);

	*domains = m_calloc(dom_num + 1, sizeof(char*));

	buf = string_new(dom_option);
	s = buf;
	i = 0;
	while ((res = strtok(s, DOMAIN_OPTION_DELIM)) != NULL) {
		(*domains)[i++] = string_new(res);

		MULK_NOTE((_("Add domain to list: %s\n"), res));
		s = NULL;
	}

	string_free(buf);
	return MULK_RET_OK;
}

int is_host_compatible_with_domains(UriUriA *uri)
{
	char *host;
	int res;

	if (!uri)
		return 0;

	if ((host = get_host(uri)) == NULL)
		return 0; 

	if (is_host_equal_domains(host, default_domains)) {
		res = 1;
		goto Exit;
	}

	if (!option_values.span_hosts) {
		res = 0;
		goto Exit;
	}

	res = ((is_domain_empty(accepted_domains) || is_host_in_domains(host, accepted_domains))
		&& !is_host_in_domains(host, rejected_domains));

Exit:
	string_free(host);

	return res;
}

static mulk_type_return_t set_option_domains(void)
{
	return parse_domains(option_values.domains, &accepted_domains);
}

static mulk_type_return_t set_option_exclude_domains(void)
{
	return parse_domains(option_values.exclude_domains, &rejected_domains);
}

static void free_domain(char ***domains)
{
	int i;

	if (!domains || !*domains)
		return;

	for (i = 0; (*domains)[i]; i++)
		string_free((*domains)[i]);

	if (*domains) {
		m_free(*domains);
		*domains = NULL;
	}
}

static void free_domains(void)
{
	free_domain(&default_domains);
	free_domain(&accepted_domains);
	free_domain(&rejected_domains);
}
#endif /* ENABLE_RECURSION */

#ifdef ENABLE_METALINK
/* Metalink locations options */

static mulk_type_return_t save_locations(const char *loc_option, char **locations)
{
	char *buf = NULL;
	char *s, *res, *ptr, *start;

	string_free(*locations);

	if (!locations || !loc_option || !*loc_option)
		return MULK_RET_OK;

	ptr = start = string_new(loc_option);
	buf = string_new(loc_option);

	s = buf;
	while ((res = strtok(s, LOC_OPTION_DELIM))) {
		if (strlen(res) != LOC_LENGTH) {
			MULK_ERROR((_("ERROR: Metalink location length is wrong: %s\n"), res));
			*locations = NULL;
			string_free(ptr);
			string_free(buf);
			return MULK_RET_OPTION_VALUE_ERR;
		}

		strcpy(ptr, res);
		ptr += LOC_LENGTH;
		MULK_NOTE((_("Add Metalink location to list: %s\n"), res));
		s = NULL;
	}

	*locations = start;
	string_free(buf);
	return MULK_RET_OK;
}

int is_location_in_list(const char *loc)
{
	char *ptr;
	country_code_t *country;

	/* if there aren't locations then the test is true */
	if ((!location_countries || !*location_countries)
	   	&& (!location_continents || !*location_continents))
		return 1;

	if (!loc || strlen(loc) != LOC_LENGTH)
		return 0;

	if (location_countries && *location_countries) {
		for (ptr = location_countries; *ptr && string_ncasecmp(ptr, loc, LOC_LENGTH); 
			ptr += LOC_LENGTH);

		/* country found */
		if (*ptr)
			return 1;
	}

	if (location_continents && *location_continents) {
		/* find continent */
		for (country = countries; country->continent_code 
			&& string_ncasecmp(country->two_letter_code, loc, LOC_LENGTH); country++);

		if (!country->continent_code)
			return 0;

		for (ptr = location_continents; *ptr && string_ncasecmp(ptr, country->continent_code, LOC_LENGTH);
			ptr += LOC_LENGTH);

		/* country found */
		if (*ptr)
			return 1;
	}

	return 0;
}

static mulk_type_return_t set_option_metalink_filename(void)
{
	return mulk_add_new_metalink_file(option_values.metalink_filename);
}

static mulk_type_return_t set_option_metalink_list_filename(void)
{
	return read_metalink_list_from_text_file(option_values.metalink_list_filename);
}

static mulk_type_return_t set_option_metalink_location(void)
{
	return save_locations(option_values.metalink_location, &location_countries);
}

static mulk_type_return_t set_option_metalink_continent(void)
{
	return save_locations(option_values.metalink_continent, &location_continents);
}

#ifdef ENABLE_CHECKSUM
static mulk_type_return_t set_option_metalink_resume_file(void)
{
	resume_file_used = 0;

	return MULK_RET_OK;
}
#endif /* ENABLE_CHECKSUM */

static void free_locations(void)
{
	string_free(location_countries);
	string_free(location_continents);
}
#endif /* ENABLE_METALINK */

static mulk_type_return_t add_directory_separ(char **dir)
{
	if (!dir || !*dir)
		return MULK_RET_OPTION_VALUE_ERR; 

	if (**dir && ((*dir)[strlen(*dir)-1] != *DIR_SEPAR_STR))
		string_cat(dir, DIR_SEPAR_STR);

	return MULK_RET_OK; 
}

static mulk_type_return_t set_option_option_filename(void)
{
	return read_option_from_text_file(option_values.option_filename);
}

static mulk_type_return_t set_option_url_filename(void)
{
	return read_uri_from_text_file(option_values.url_filename);
}

static mulk_type_return_t set_option_report_filename(void)
{
	if (option_values.report_filename && *option_values.report_filename) {
		if (is_file_exist(option_values.report_filename) && remove(option_values.report_filename))
			return MULK_RET_FILE_ERR;
		else
			return MULK_RET_OK;
	}
	else
		return MULK_RET_FILE_ERR;
}

static mulk_type_return_t set_option_report_csv_filename(void)
{
	if (option_values.report_csv_filename && *option_values.report_csv_filename) {
		if (is_file_exist(option_values.report_csv_filename) && remove(option_values.report_csv_filename))
			return MULK_RET_FILE_ERR;
		else
			return MULK_RET_OK;
	}
	else
		return MULK_RET_FILE_ERR;
}

static mulk_type_return_t set_option_mime_output_directory(void)
{
	return add_directory_separ(&option_values.mime_output_directory);
}

static mulk_type_return_t set_option_file_output_directory(void)
{
	return add_directory_separ(&option_values.file_output_directory);
}

static mulk_type_return_t set_option_temp_directory(void)
{
	mulk_type_return_t ret = add_directory_separ(&option_values.temp_directory);
	
	if (is_file_exist(option_values.temp_directory) && remove_dir(option_values.temp_directory))
		MULK_WARN((_("WARNING: removing temporary directory\n")));

	return  ret;
}

/* general options */
static struct option *create_long_options_array(void)
{
	int i;
	struct option *long_opts;

	long_opts = m_calloc(NUM_OPTIONS + 1, sizeof(struct option));

	for (i = 0; i < NUM_OPTIONS; i++) {
		long_opts[i].name = options[i].long_opt;
		long_opts[i].has_arg = (options[i].type == OPTION_INT || options[i].type == OPTION_STRING)
			? required_argument : no_argument;
		long_opts[i].flag = NULL;
		long_opts[i].val = options[i].short_opt;

		MULK_DEBUG((_("long option inserted %d, %s\n"), i, long_opts[i].name));
	}

	return long_opts;
}

static char *create_short_options_string(struct option *long_opts)
{
	int i, count = 0;
	char *short_opts, *ptr;

	for (i = 0; long_opts[i].name; i++)
		if (long_opts[i].val) {
			count++;
			if (long_opts[i].has_arg == required_argument)
				count++;
		}

	short_opts = ptr = string_alloc(count);

	for (i = 0; long_opts[i].name; i++)
		if (long_opts[i].val) {
			*(ptr++) = (char) long_opts[i].val;
			if (long_opts[i].has_arg == required_argument)
				*(ptr++) = ':';
		}

	MULK_DEBUG((_("short option string %s\n"), short_opts));
	return short_opts;
}

void mulk_printf_usage(void)
{
	int i;

#ifdef ENABLE_METALINK
	if (option_values.metalink_print_locations) {
		printf_locations();
		return;
	}
	if (option_values.metalink_print_continents) {
		printf_continents();
		return;
	}

	printf(_("Mulk %s, a non-interactive multi-connection network downloader with image filtering and Metalink support.\n"),
		mulk_version_number);
#else
	printf(_("Mulk %s, a non-interactive multi-connection network downloader with image filtering.\n"),
		mulk_version_number);
#endif
	printf(_("Usage: Mulk [OPTION]... [URL]...\n"));

	for (i = 0; i < NUM_OPTIONS; i++) {
		if (options[i].title)
			printf("\n%s:\n", _(options[i].title));

		if (options[i].short_opt)
			printf("  -%c,", options[i].short_opt);
		else
			printf("     ");

		printf(" --%-27s%s\n", options[i].long_opt, _(options[i].description));
	}

	printf("\n");
}

void mulk_printf_version(void)
{
	printf(_("Mulk version %s\n"), mulk_version_number);
	printf(_("\nMulk  Copyright (C) 2008, 2009, 2010, 2011, 2012  Emanuele Bovisio\n"));
	printf(_("This program comes with ABSOLUTELY NO WARRANTY.\n"));
	printf(_("This is free software, and you are welcome to redistribute it\n"));
	printf(_("under certain conditions.\n"));
	printf(_("See the GNU General Public License version 3 or later\n"));
	printf(_("for more details.\n\n"));
}

mulk_type_return_t mulk_find_short_option(const char name, int *opt_index)
{
	int i;

	if (!name)
		return MULK_RET_ERR;
	
	for (i = 0; i < NUM_OPTIONS && options[i].short_opt != name; i++);

	if (i >= NUM_OPTIONS) {
		fprintf(stderr, _("\nERROR: option not recognised\n\n"));
		return MULK_RET_OPTION_ERR;
	}

	if (opt_index)
		*opt_index = i;

	return MULK_RET_OK;
}

mulk_type_return_t mulk_find_long_option(const char *name, int *opt_index)
{
	int i;

	if (!name || !*name)
		return MULK_RET_ERR;

	for (i = 0; i < NUM_OPTIONS && strcmp(options[i].long_opt, name); i++);

	if (i >= NUM_OPTIONS) {
		fprintf(stderr, _("\nERROR: option not recognised\n\n"));
		return MULK_RET_OPTION_ERR;
	}

	if (opt_index)
		*opt_index = i;

	return MULK_RET_OK;
}

void init_options(void)
{
#ifdef ENABLE_RECURSION
	option_values.depth = 1;
#endif
	option_values.max_sim_conns_per_host = 2;
	option_values.max_sim_conns = 50;

	option_values.report_every_lines = 500;

	option_values.mime_output_directory = string_new(DEFAULT_MIME_OUTPUT_DIRECTORY);
	set_option_mime_output_directory();
	option_values.file_output_directory = string_new(DEFAULT_FILE_OUTPUT_DIRECTORY);
	set_option_file_output_directory();
	option_values.temp_directory = string_new(DEFAULT_TEMP_DIRECTORY);
	set_option_temp_directory();

#ifdef ENABLE_CHECKSUM
	resume_file_used = 0;
#endif
}

mulk_type_return_t mulk_set_option(int ind, const char *value)
{
	int val;
	char *name;

	if (ind < 0 || ind >= NUM_OPTIONS) {
		fprintf(stderr, _("\nERROR: option not recognised\n\n"));
		return MULK_RET_OPTION_ERR;
	}

	name = options[ind].long_opt;

	MULK_DEBUG((_("option recognised %d, %s=%s\n"), ind, name, value ? value : ""));

	switch (options[ind].type) {
		case OPTION_BOOL:
			*((int*) options[ind].value) = 1;
			break;
		case OPTION_INT:
			if (!value) {
				fprintf(stderr, _("\nERROR: this option (%s) needs to be followed by a value\n\n"), name ? name : "");
				return MULK_RET_OPTION_VALUE_ERR;
			}
			val = atoi(value);
			if (val < options[ind].min_val || val > options[ind].max_val) {
				fprintf(stderr, _("\nERROR: %s\n\n"), _(options[ind].error));
				return MULK_RET_OPTION_VALUE_ERR;
			}
			*((int*) options[ind].value) = val;
			break;
		case OPTION_STRING:
			if (!value) {
				fprintf(stderr, _("\nERROR: this option (%s) needs to be followed by a value\n\n"), name ? name : "");
				return MULK_RET_OPTION_VALUE_ERR;
			}
			string_free(*((char**) options[ind].value));

			*((char**) options[ind].value) = string_new(value);
			break;
		case OPTION_HELP:
			if ((int*) options[ind].value)
				*((int*) options[ind].value) = 1;
			return MULK_RET_HELP;
		case OPTION_VERSION:
			return MULK_RET_VERSION;
		default:
			fprintf(stderr, _("\nERROR: option type not valid, %d\n\n"), options[ind].type);
			return MULK_RET_ERR;
	}

	if (options[ind].set_option_value)
		return options[ind].set_option_value();

	return MULK_RET_OK;
}

mulk_type_return_t mulk_set_options(int argc, char **argv)
{
	struct option *long_options = create_long_options_array();
	char *short_options = create_short_options_string(long_options);
	int c, option_index;
	mulk_type_return_t ret = MULK_RET_OK;

	for (;;) {
		option_index = 0;

		c = getopt_long(argc, argv, short_options, long_options, &option_index);

		/* option finished */
		if (c == -1)
			break;

		/* not recognised */
		if (c == '?') {
			ret = MULK_RET_OPTION_ERR;
			goto Exit;
		}

		/* short option */
		if (c && ((ret = mulk_find_short_option((char) c, &option_index)) != MULK_RET_OK)) 
			goto Exit;

		if ((ret = mulk_set_option(option_index, optarg)) != MULK_RET_OK) 
			goto Exit;
	}

	for (; optind < argc; optind++) {
		if ((ret = mulk_add_new_url(argv[optind])) == MULK_RET_OK)
			MULK_NOTE((_("Add url to download coming from command line: %s\n"), argv[optind]));
		else {
			fprintf(stderr, _("\nERROR: wrong url: %s\n\n"), argv[optind]);
			goto Exit;
		}
	}

	if (is_url_list_empty()) {
		fprintf(stderr, _("\nERROR: url not present\n\n"));
		ret = MULK_RET_URL_ERR;
	}

Exit:
	if (ret < MULK_RET_OK)
		fprintf(stderr, _("Try `mulk --help' for more information.\n"));

	string_free(short_options);
	m_free(long_options);

	return ret;
}

MULK_API mulk_type_return_t mulk_set_short_option(const char name, const char *value)
{
	mulk_type_return_t ret = MULK_RET_OK;
	int opt_index = 0;

	if ((ret = mulk_find_short_option(name, &opt_index)) != MULK_RET_OK)
		return ret;

	return mulk_set_option(opt_index, value);
}

MULK_API mulk_type_return_t mulk_set_long_option(const char *name, const char *value)
{
	mulk_type_return_t ret = MULK_RET_OK;
	int opt_index = 0;

	if ((ret = mulk_find_long_option(name, &opt_index)) != MULK_RET_OK)
		return ret;

	return mulk_set_option(opt_index, value);
}

void free_options(void)
{
	int i;

	for (i = 0; i < NUM_OPTIONS; i++)
		if (options[i].type == OPTION_STRING)
			string_free(*((char **) options[i].value));

#ifdef ENABLE_RECURSION
	free_domains();
#endif
#ifdef ENABLE_METALINK
	free_locations();
#endif
}

void mulk_reset_options(void)
{
	int i;

	for (i = 0; i < NUM_OPTIONS; i++) {
		switch (options[i].type) {
			case OPTION_BOOL:
			case OPTION_INT:
			case OPTION_HELP:
				if ((int*) options[i].value)
					*((int*) options[i].value) = 0;
				break;
			case OPTION_STRING:
				string_free(*((char **) options[i].value));
				break;
			default:
				break;
		}
	}

#ifdef ENABLE_RECURSION
	free_domain(&accepted_domains);
	free_domain(&rejected_domains);
#endif /* ENABLE_RECURSION */

#ifdef ENABLE_METALINK
	free_locations();
#endif

	init_options();
}
