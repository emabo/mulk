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

#ifndef _DEFINES_H_
#define _DEFINES_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H 
#include <inttypes.h>
#endif
#include <curl/curl.h>
#include <curl/multi.h>
#include <uriparser/Uri.h>
#include <mulk/mulk.h>

/* define DEBUG compilation flags */
#if (defined(DEBUG) || defined(_DEBUG)) && !defined(NDEBUG)
#define MULKDEBUG 1
#endif

#include "m_malloc.h"
#include "uri_parser.h"
#include "option_obj.h"
#include "gettext.h"

#define _(text) gettext(text)

/* http protocols */
#define HTTP_PROTOCOL "http"
#define HTTPS_PROTOCOL "https"
/* ftp protocol */
#define FTP_PROTOCOL "ftp"
/* scheme separator */
#define SCHEME_SEPAR_STR "://"

/* log levels */
#define MERR 0
#define MWAR 1
#define MNOTE 2
#define MINFO 3

/* win32 specific definitions */
#ifdef _WIN32
/* remove annoying compilation warning under MSVC */
#define REMOVE_4127_WARNING __pragma(warning(disable:4127))

typedef UINT32 uint32_t;
typedef UINT32 intmax_t;

#define PRIdMAX "d"

#define VERSION "0.7.0"
#define DIR_SEPAR_STR "\\"

#define mulk_fseek fseek

#else
#define REMOVE_4127_WARNING

#define DIR_SEPAR_STR "/"

#ifdef HAVE_FSEEKO
#define mulk_fseek fseeko
#else
#define mulk_fseek fseek
#endif

#endif

#define GET_SEPAR(out_dir) \
	((!*(out_dir) || (out_dir)[strlen(out_dir)-1] == *DIR_SEPAR_STR) ? "" : DIR_SEPAR_STR)

#define is_printf(log_level) \
	(option_values.verbosity || (!option_values.quiet && (log_level) < MINFO))

#define MULK_ERROR(args) \
REMOVE_4127_WARNING \
do {if (is_printf(MERR)) printf args;} while (0)

#define MULK_WARN(args) \
REMOVE_4127_WARNING \
do {if (is_printf(MWAR)) printf args;} while (0)

#define MULK_NOTE(args) \
REMOVE_4127_WARNING \
do {if (is_printf(MNOTE)) printf args;} while (0)

#define MULK_INFO(args) \
REMOVE_4127_WARNING \
do {if (is_printf(MINFO)) printf args;} while (0)

#ifdef MULKDEBUG
#define MULK_DEBUG(args) \
REMOVE_4127_WARNING \
do {printf("%s,%d: ", __FILE__, __LINE__); printf args;} while (0)
#else
#define MULK_DEBUG(args)
#endif

#endif /* not _DEFINES_H_ */
