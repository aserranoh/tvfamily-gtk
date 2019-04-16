/*
coretypes.c - Types defined by the TVFamily API.

This file is part of tvfamily-gtk.

Copyright 2019 Antonio Serrano Hernandez

tvfamily-gtk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

tvfamily-gtk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tvfamily-gtk; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.
*/

#include <err.h>
#include <glib.h>

#include "coretypes.h"

/* Load a string attribute from a json node. */
static char *
media_load_string_attr (Media *m, json_t *j, const char *attr)
{
    json_t *a;
    char *val;

    a = json_object_get (j, attr);
    if (!json_is_string (a)) {
        val = g_strdup ("");
    } else {
        val = g_strdup (json_string_value (a));
    }
    return val;
}

/* Load an integer attribute from a json node. */
int
media_load_int_attr (Media *m, json_t *j, const char *attr)
{
    json_t *a;
    int val;

    a = json_object_get (j, attr);
    if (!json_is_integer (a)) {
        val = -1;
    } else {
        val = json_integer_value (a);
    }
    return val;
}

/* Load a string list from a json node. */
static char **
media_load_string_list_attr (Media *m, json_t *j, const char *attr)
{
    json_t *a, *elem;
    char **val;

    a = json_object_get (j, attr);
    if (!json_is_array (a)) {
        val = g_new (char *, 1);
        val[0] = NULL;
    } else {
        size_t len = json_array_size (a);
        val = g_new (char *, len + 1);
        for (int i = 0; i < len; i++) {
            elem = json_array_get (a, i);
            if (!json_is_string (elem)) {
                val[i] = g_strdup ("");
            } else {
                val[i] = g_strdup (json_string_value (elem));
            }
        }
        val[len] = NULL;
    }
    return val;
}

Media *
media_new (json_t *j)
{
    json_t *x;
    
    Media *m = g_new (Media, 1);
    // Read the title_id attribute
    m->title_id = media_load_string_attr (m, j, "title_id");
    // Read the title attribute
    m->title = media_load_string_attr (m, j, "title");
    // Read the year attribute
    m->air_year = media_load_int_attr (m, j, "air_year");
    // Read the genre attribute
    m->genre = media_load_string_list_attr (m, j, "genre");
    // Read the rating attribute
    m->rating = media_load_string_attr (m, j, "rating");
    // Read the rating attribute
    m->season = media_load_int_attr (m, j, "season");
    // Read the rating attribute
    m->episode = media_load_int_attr (m, j, "episode");
    return m;
}

char *
media_to_string (Media *m)
{
    char *s;

    if (m->season < 0 && m->episode < 0) {
        s = g_strdup (m->title);
    } else {
        s = g_strdup_printf ("%s %dx%02d", m->title, m->season, m->episode);
    }
    return s;
}

void
media_destroy (Media *m)
{
    g_free (m->title_id);
    g_free (m->title);
    for (int i = 0; m->genre[i]; i++) {
        g_free (m->genre[i]);
    }
    g_free (m->genre);
    g_free (m->rating);
    g_free (m);
}

