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

#ifndef _MULK_H_
#define _MULK_H_

#if defined(_WIN32) || defined(__CYGWIN__)
	#define MULK_DLL_IMPORT __declspec(dllimport)
	#define MULK_DLL_EXPORT __declspec(dllexport)
	#define MULK_DLL_LOCAL
#else /* not WIN32 and not CYGWIN */
	#if defined(__GNUC__) && (__GNUC__ >= 4)
		#define MULK_DLL_IMPORT __attribute__ ((visibility("default")))
		#define MULK_DLL_EXPORT __attribute__ ((visibility("default")))
		#define MULK_DLL_LOCAL  __attribute__ ((visibility("hidden")))
	#else
		#define MULK_DLL_IMPORT
		#define MULK_DLL_EXPORT
		#define MULK_DLL_LOCAL
	#endif
#endif /* not WIN32 and not CYGWIN */


/*
 * Compilation flags definitions:
 *
 * * library compilation
 * --------------------------------------------------------
 * lib. type:   shared/dll                static
 * comp. flag:  PIC & BUILDING_MULK_LIB   BUILDING_MULK_LIB
 * --------------------------------------------------------
 *
 * * executable linked against
 * --------------------------------------------------------
 * lib. type:   shared/dll                static
 * comp. flag:  -                         MULK_STATIC_LIB
 * --------------------------------------------------------
*/
#if defined(MULK_STATIC_LIB) || (defined(BUILDING_MULK_LIB) && !defined(PIC)) /* static lib */ 
	#define MULK_API
	#define MULK_LOCAL
#else /* shared lib */
	#ifdef BUILDING_MULK_LIB
		#define MULK_API MULK_DLL_EXPORT
	#else
		#define MULK_API MULK_DLL_IMPORT
	#endif 
	#define MULK_LOCAL MULK_DLL_LOCAL
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mulk_type_return_t {
	MULK_RET_OPTION_ERR = -5,
	MULK_RET_OPTION_VALUE_ERR = -4,
	MULK_RET_URL_ERR = -3,
	MULK_RET_FILE_ERR = -2,
	MULK_RET_ERR = -1,
	MULK_RET_OK = 0,
	MULK_RET_EXIT = 1,
	MULK_RET_HELP = 2,
	MULK_RET_VERSION = 3,
} mulk_type_return_t;

typedef int (*mulk_check_exit_cb)(void *context);

typedef void (*mulk_write_download_info_cb)(void *context, int ret_code, const char *result,
	int is_http, long http_code, const char *mime_type, const char *url, long long int size);

extern char mulk_version_number[];

/* initialisation */
MULK_API mulk_type_return_t mulk_find_short_option(const char name, int *index);
MULK_API mulk_type_return_t mulk_find_long_option(const char *name, int *index);
MULK_API mulk_type_return_t mulk_set_option(int index, const char *value);
MULK_API mulk_type_return_t mulk_set_short_option(const char name, const char *value);
MULK_API mulk_type_return_t mulk_set_long_option(const char *name, const char *value);
MULK_API mulk_type_return_t mulk_set_options(int argc, char **argv);
MULK_API mulk_type_return_t mulk_add_new_url(const char *url);
/* defined only if --enable-metalink is specified when compiling library, 
 * otherwise this function returns option error */
MULK_API mulk_type_return_t mulk_add_new_metalink_file(const char *metalink_filename);

/* print info */
MULK_API void mulk_printf_usage(void);
MULK_API void mulk_printf_version(void);

/* callbacks */
MULK_API void mulk_set_check_exit_cb(mulk_check_exit_cb cb, void *context);
MULK_API void mulk_set_write_download_info_cb(mulk_write_download_info_cb cb, void *context);

/* main loop */
MULK_API void mulk_init(void);
MULK_API mulk_type_return_t mulk_run(void);
MULK_API void mulk_close(void);

/* restart process */
MULK_API void mulk_reset_options(void);

#ifdef __cplusplus
}
#endif

#endif /* not _MULK_H_ */
