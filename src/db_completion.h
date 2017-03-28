/* ncmpc (Ncurses MPD Client)
 * (c) 2004-2017 The Music Player Daemon Project
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

#ifndef DB_COMPLETION_H
#define DB_COMPLETION_H

#include <glib.h>

struct mpdclient;

#define GCMP_TYPE_DIR       (0x01 << 0)
#define GCMP_TYPE_FILE      (0x01 << 1)
#define GCMP_TYPE_PLAYLIST  (0x01 << 2)
#define GCMP_TYPE_RFILE     (GCMP_TYPE_DIR | GCMP_TYPE_FILE)
#define GCMP_TYPE_RPLAYLIST (GCMP_TYPE_DIR | GCMP_TYPE_PLAYLIST)

/**
 * Create a list suitable for GCompletion from path.
 */
GList *
gcmp_list_from_path(struct mpdclient *c, const gchar *path,
		    GList *list, gint types);

#endif