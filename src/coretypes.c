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
json_load_string_attr (json_t *j, const char *attr)
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
json_load_int_attr (json_t *j, const char *attr)
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

Media *
media_new (json_t *j)
{
    Media *m = g_new (Media, 1);
    // Read the title_id attribute
    m->title_id = json_load_string_attr (j, "title_id");
    // Read the title attribute
    m->title = json_load_string_attr (j, "title");
    // Read the rating attribute
    m->rating = json_load_string_attr (j, "rating");
    // Read the rating attribute
    m->season = json_load_int_attr (j, "season");
    // Read the rating attribute
    m->episode = json_load_int_attr (j, "episode");
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

gboolean
media_equal (Media *m1, Media *m2)
{
    return g_strcmp0 (m1->title_id, m2->title_id) == 0
        && m1->season == m2->season && m1->episode == m2->episode;
}

void
media_destroy (Media *m)
{
    g_free (m->title_id);
    g_free (m->title);
    g_free (m->rating);
    g_free (m);
}

/*Title *
title_new (json_t *j)
{
    Title *t = g_new (Title, 1);
    // Read the title_id attribute
    t->title_id = json_load_string_attr (j, "title_id");
    // Read the title attribute
    t->title = json_load_string_attr (j, "title");
    // Read the rating attribute
    t->rating = json_load_string_attr (j, "rating");
    return m;
}*/

