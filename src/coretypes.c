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

Media *
media_new (json_t *j)
{
    json_t *x;

    Media *m = g_new (Media, 1);
    // Read the title_id attribute
    x = json_object_get (j, "title_id");
    if (!json_is_string (x)) {
        warnx ("media_new: not valid title_id attribute");
        m->title_id = g_strdup ("");
    } else {
        m->title_id = g_strdup (json_string_value (x));
    }
    return m;
}

void
media_destroy (Media *m)
{
    g_free (m->title_id);
    g_free (m);
}

