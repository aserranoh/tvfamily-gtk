/*
paths.c - Paths for different elements of the application.

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define STATIC_PATH             "data"
#define LOGO_FILE               "tvfamily.svg"
#define STYLES_FILE             "styles.css"
#define DEFAULT_PICTURE_FILE    "profile-default.svg"

static const char *logo_path = STATIC_PATH "/" LOGO_FILE;
static const char *styles_path = STATIC_PATH "/" STYLES_FILE;
static const char *default_picture_path = STATIC_PATH "/" DEFAULT_PICTURE_FILE;

const char *
paths_get_image (const char *image)
{
    size_t len = strlen (STATIC_PATH) + 6 + strlen (image);
    char *s = malloc (len);

    if (!s) {
        err (1, "paths_get_image");
    }
    snprintf (s, len, "%s/%s.svg", STATIC_PATH, image);
    return s;
}

const char *
paths_get_logo ()
{
    return logo_path;
}

const char *
paths_get_styles ()
{
    return styles_path;
}

const char *
paths_get_default_picture ()
{
    return default_picture_path;
}

