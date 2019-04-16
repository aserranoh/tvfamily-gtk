/*
coretypes.h - Types defined by the TVFamily API.

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

#ifndef CORETYPES_H
#define CORETYPES_H

#include <jansson.h>

typedef struct {
    char *title_id;
    char *title;
    int air_year;
    char **genre;
    char *rating;
    int season;
    int episode;
} Media;

/* Create a new instance of a media from its contents in json. */
Media *
media_new (json_t *j);

/* Return the string representation of this media. */
char *
media_to_string (Media *m);

/* Free all the members that were allocated inside the Media. */
void
media_destroy (Media *m);

#endif

