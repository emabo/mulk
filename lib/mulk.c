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

#include "m_malloc.h"
#include "buffer_array.h"
#include "url_list.h"
#include "option_obj.h"
#include "parse.h"
#include "jpg_obj.h"
#include "file_obj.h"
#include "string_obj.h"


char mulk_version_number[] = VERSION;
static mulk_check_exit_cb check_exit = NULL;
static void *exit_context = NULL;
static mulk_write_download_info_cb write_download_info = NULL;
static void *info_context = NULL;

void mulk_set_check_exit_cb(mulk_check_exit_cb cb, void *context)
{
	check_exit = cb;
	exit_context = context;
}

void mulk_set_write_download_info_cb(mulk_write_download_info_cb cb, void *context)
{
	write_download_info = cb;
	info_context = context;
}

#ifdef ENABLE_NLS
static void init_nls(void)
{
	setlocale(LC_ALL, "");
	bindtextdomain("mulk", LOCALEDIR);
	textdomain("mulk");
}
#endif /* ENABLE_NLS */

void mulk_init(void)
{
#ifdef MULKDEBUG
	m_debug_init();
#endif

	MULK_DEBUG((_("libcURL version %s\n"), LIBCURL_VERSION));

#ifdef ENABLE_NLS
	init_nls();
#endif

	init_options();
}

mulk_type_return_t mulk_run(void)
{
	CURLM *curl_obj;
	CURL *e;
	CURLMsg *curl_msg;
	int num_fd, num_msgs, active_handles;
	long time_to_wait, file_tot = 0;
	off_t download_tot = 0;
	fd_set read_fd, write_fd, except_fd;
	struct timeval timeout;
	time_t t1, t2, tot_time;
	mulk_type_return_t ret = MULK_RET_OK;

	(void) time(&t1);

	create_buffer_array();
	curl_global_init(CURL_GLOBAL_ALL);
	curl_obj = curl_multi_init();
	curl_multi_setopt(curl_obj, CURLMOPT_MAXCONNECTS, (long) option_values.max_sim_conns);

	for (active_handles = 0; (active_handles < option_values.max_sim_conns) && (init_url(curl_obj) == MULK_RET_OK);
		active_handles++);

	if (!active_handles)
		MULK_ERROR((_("No usable URLs for downloading\n")));

	while (active_handles) {
		while (curl_multi_perform(curl_obj, &active_handles) == CURLM_CALL_MULTI_PERFORM);

		if (active_handles) {
			FD_ZERO(&read_fd);
			FD_ZERO(&write_fd);
			FD_ZERO(&except_fd);

			if (curl_multi_fdset(curl_obj, &read_fd, &write_fd, &except_fd, &num_fd)) {
				MULK_ERROR((_("ERROR: fdset\n")));
				ret = MULK_RET_ERR;
				goto Exit;
			}

			if (check_exit && check_exit(exit_context)) {
				ret = MULK_RET_EXIT;
				goto Exit;
			}

			if (curl_multi_timeout(curl_obj, &time_to_wait)) {
				MULK_ERROR((_("ERROR: timeout\n")));
				ret = MULK_RET_ERR;
				goto Exit;
			}

			if (time_to_wait < 0)
				time_to_wait = 100;

			if (num_fd == -1)
#ifdef _WIN32
				Sleep(time_to_wait);
#else
				sleep(time_to_wait / 1000);
#endif
			else {
				timeout.tv_sec = time_to_wait / 1000;
				timeout.tv_usec = (time_to_wait % 1000) * 1000;

				if (select(num_fd+1, &read_fd, &write_fd, &except_fd, &timeout) < 0) {
					MULK_ERROR((_("ERROR: num_fd:%d, time_to_wait %li, error message %s (%d)\n"),
						num_fd+1, time_to_wait, strerror(errno), errno));
					ret = MULK_RET_ERR;
					goto Exit;
				}
			}
		}

		do {
			e = NULL;
			while ((curl_msg = curl_multi_info_read(curl_obj, &num_msgs)) != NULL) {
				e = curl_msg->easy_handle;
				break;
			}
			if (e) {
				if (curl_msg->msg == CURLMSG_DONE) {
					char *mime_type = NULL, *orig_url = NULL;
					long resp_code = -1;
					double download_size = 0;
					int is_http, is_file_completed = 0;
					CURLcode err_code = curl_msg->data.result;
					const char *err_str = curl_easy_strerror(err_code);

					if (curl_easy_getinfo(e, CURLINFO_PRIVATE, &orig_url) != CURLE_OK)
						orig_url = NULL;
					if (curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &resp_code) != CURLE_OK)
						resp_code = 0;
					if (curl_easy_getinfo(e, CURLINFO_SIZE_DOWNLOAD, &download_size) != CURLE_OK)
						download_size = 0;

					download_tot += (off_t) download_size;

					MULK_NOTE((_("RESULT: %d (%s), "), err_code, err_str ? err_str : ""));
					if ((is_http = is_uri_http_buffer(e)) != 0) {
						if (curl_easy_getinfo(e, CURLINFO_CONTENT_TYPE, &mime_type) != CURLE_OK)
							mime_type = NULL;
						MULK_NOTE((_("HTTP Code:\"%ld\" Mime-Type:\"%s\" "), resp_code, mime_type ? mime_type : ""));
						set_buffer_mime_type(e, mime_type);
					}
					else
						mime_type = NULL;
					MULK_NOTE((_("Url:\"%s\" Size:%" PRIdMAX "\n"), orig_url ? orig_url : "", (intmax_t) download_size));

					if (write_download_info)
						write_download_info(info_context, err_code, err_str, is_http, resp_code, mime_type,
							orig_url, (long long int) download_size);

					if (close_buffer(e, err_code, resp_code, &is_file_completed) == MULK_RET_OK)
						file_tot += is_file_completed;

					curl_multi_remove_handle(curl_obj, e);
					curl_easy_cleanup(e);
					string_free(orig_url);
				}
				else {
					int is_file_completed;

					if (close_buffer(e, CURLE_FAILED_INIT, 0, &is_file_completed) == MULK_RET_OK)
						file_tot += is_file_completed;

					MULK_ERROR((_("ERROR: message=%d\n"), curl_msg->msg));
				}

				if ((option_values.report_filename || option_values.report_csv_filename)
					&& option_values.report_every_lines) {
					if ((file_tot % option_values.report_every_lines) == 0)
						report_urls();
				}
			}
		} while (e);

		while ((active_handles < option_values.max_sim_conns) && (init_url(curl_obj) == MULK_RET_OK))
			active_handles++;
	}

Exit:
	free_buffer_array(curl_obj);
	curl_multi_cleanup(curl_obj);
	curl_global_cleanup();

	(void) time(&t2);
	tot_time = t2 - t1;

	MULK_NOTE((_("\nTotal time to download = %ld seconds"), (long int) tot_time));
	MULK_NOTE((_("\nTotal downloaded files = %ld"), (long int) file_tot));
	MULK_NOTE((_("\nTotal size downloaded = %.2f KB\n"), (double) download_tot / 1000.0f));
	if (tot_time > 0)
		MULK_NOTE((_("\nAverage data rate = %.2f KB/s\n"), ((double) download_tot) / (tot_time * 1000.0f)));

	if (option_values.report_filename || option_values.report_csv_filename)
		report_urls();

	return ret;
}

void mulk_close(void)
{
	free_urls();
#ifdef ENABLE_METALINK
	free_metalinks();
#endif
	free_options();

#ifdef MULKDEBUG
	m_print_allocated_pointers();
#endif
}
