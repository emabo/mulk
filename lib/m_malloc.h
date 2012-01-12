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

#ifndef _M_MALLOC_H_
#define _M_MALLOC_H_

#include "defines.h"

#define default_free free

void *default_calloc(size_t nmemb, size_t size);

#ifdef MULKDEBUG

#define m_calloc(m, s) debug_calloc(m, s, __FILE__, __LINE__)
#define m_free(p)      debug_free(p, __FILE__, __LINE__)

void *debug_calloc(size_t nmemb, size_t size, const char *file, int line);
void debug_free(void *ptr, const char *file, int line);

void m_debug_init(void);
void m_print_pointers_distribution(void);
void m_print_allocated_pointers(void);

#else /* not MULKDEBUG */

#define m_calloc default_calloc
#define m_free   default_free

#endif /* not MULKDEBUG */
#endif /* not _M_MALLOC_H_ */
