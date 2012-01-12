/*---------------------------------------------------------------------------
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 - Emanuele Bovisio
 *
 * This file is part of Mulk.
 *
 * Mulk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mulk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mulk.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 *---------------------------------------------------------------------------*/

#include <stdio.h> 
#include <stdlib.h> 
#include <mulk/mulk.h>


int main(int argc, char **argv)
{
	mulk_type_return_t opt_ret;
	int ret = EXIT_SUCCESS;

	mulk_init();

	opt_ret = mulk_set_options(argc, argv);

	if (opt_ret == MULK_RET_OK) {
		if (mulk_run() != MULK_RET_OK)
			ret = EXIT_FAILURE;
	}
	else if (opt_ret == MULK_RET_HELP)
		mulk_printf_usage();
	else if (opt_ret == MULK_RET_VERSION)
		mulk_printf_version();
	else 
		ret = EXIT_FAILURE;

	mulk_close();
	exit(ret);
}

