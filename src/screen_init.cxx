/* ncmpc (Ncurses MPD Client)
 * (c) 2004-2018 The Music Player Daemon Project
 * Project homepage: http://musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "screen.hxx"
#include "screen_interface.hxx"
#include "screen_list.hxx"
#include "screen_queue.hxx"
#include "config.h"
#include "i18n.h"
#include "utils.hxx"
#include "options.hxx"
#include "colors.hxx"

#include <stdlib.h>

#ifndef NCMPC_MINI
/** welcome message time [s] */
static const GTime SCREEN_WELCOME_TIME = 10;
#endif

/* minimum window size */
static const unsigned SCREEN_MIN_COLS = 14;
static const unsigned SCREEN_MIN_ROWS = 5;

void
ScreenManager::Exit()
{
	if (current_page->close != nullptr)
		current_page->close();

	screen_list_exit();

	string_list_free(find_history);
	g_free(buf);
	g_free(findbuf);

	title_bar_deinit(&title_bar);
	delwin(main_window.w);
	progress_bar_deinit(&progress_bar);
	status_bar_deinit(&status_bar);

#ifndef NCMPC_MINI
	if (welcome_source_id != 0)
		g_source_remove(welcome_source_id);
#endif
}

void
ScreenManager::OnResize(struct mpdclient *c)
{
	const unsigned cols = COLS, rows = LINES;
	if (cols < SCREEN_MIN_COLS || rows < SCREEN_MIN_ROWS) {
		Exit();
		fprintf(stderr, "%s\n", _("Error: Screen too small"));
		exit(EXIT_FAILURE);
	}

#ifdef PDCURSES
	resize_term(rows, cols);
#else 
	resizeterm(rows, cols);
#endif

	title_bar_resize(&title_bar, cols);

	/* main window */
	main_window.cols = cols;
	main_window.rows = rows - 4;
	wresize(main_window.w, main_window.rows, cols);

	/* progress window */
	progress_bar_resize(&progress_bar, cols, rows - 2, 0);

	/* status window */
	status_bar_resize(&status_bar, cols, rows - 1, 0);

	buf_size = cols;
	g_free(buf);
	buf = (char *)g_malloc(cols);

	/* resize all screens */
	screen_list_resize(main_window.cols, main_window.rows);

	/* ? - without this the cursor becomes visible with aterm & Eterm */
	curs_set(1);
	curs_set(0);

	Paint(c, true);
}

#ifndef NCMPC_MINI
static gboolean
welcome_timer_callback(gpointer data)
{
	auto *c = (struct mpdclient *)data;

	screen.welcome_source_id = 0;

	screen.PaintTopWindow(c);
	doupdate();

	return false;
}
#endif

void
ScreenManager::Init(struct mpdclient *c)
{
	const unsigned cols = COLS, rows = LINES;
	if (cols < SCREEN_MIN_COLS || rows < SCREEN_MIN_ROWS) {
		fprintf(stderr, "%s\n", _("Error: Screen too small"));
		exit(EXIT_FAILURE);
	}

	current_page = &screen_queue;
	buf = (char *)g_malloc(cols);
	buf_size = cols;
	findbuf = nullptr;

#ifndef NCMPC_MINI
	if (options.welcome_screen_list)
		welcome_source_id =
			g_timeout_add_seconds(SCREEN_WELCOME_TIME,
					      welcome_timer_callback, c);
#endif

	/* create top window */
	title_bar_init(&title_bar, cols, 0, 0);

	/* create main window */
	window_init(&main_window, rows - 4, cols, 2, 0);

	if (!options.hardware_cursor)
		leaveok(main_window.w, true);

	keypad(main_window.w, true);

	/* create progress window */
	progress_bar_init(&progress_bar, cols, rows - 2, 0);

	/* create status window */
	status_bar_init(&status_bar, cols, rows - 1, 0);

#ifdef ENABLE_COLORS
	if (options.enable_colors) {
		/* set background attributes */
		wbkgd(stdscr, COLOR_PAIR(COLOR_LIST));
		wbkgd(main_window.w,     COLOR_PAIR(COLOR_LIST));
		wbkgd(title_bar.window.w, COLOR_PAIR(COLOR_TITLE));
		wbkgd(progress_bar.window.w,
		      COLOR_PAIR(COLOR_PROGRESSBAR));
		wbkgd(status_bar.window.w, COLOR_PAIR(COLOR_STATUS));
		colors_use(progress_bar.window.w, COLOR_PROGRESSBAR);
	}
#endif

	/* initialize screens */
	screen_list_init(main_window.w, main_window.cols, main_window.rows);

	if (current_page->open != nullptr)
		current_page->open(c);
}