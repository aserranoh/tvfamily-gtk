/*
core.h - Application's core API headers.

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

#ifndef CORE_H
#define CORE_H

#include <curl/curl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "coretypes.h"
#include "requests.h"

// Core main data structure
typedef struct Core_s {
    char *server_address;
    char *profile;
} Core;

// Definition of the core global variable
extern Core core;

/* Core structures initialization. */
void
core_create(const char *server_address);

/* Set the current profile. */
void
core_set_profile (const char *profile);

/* Request the list of profiles and execute the callback when done. */
void
core_request_profiles (profiles_request_callback callback);

/* Request a profile picture. */
void
core_request_profile_picture (const char *profile,
                              picture_request_callback callback);

/* Create a profile. */
int
core_create_profile (const char *name, GdkPixbuf *picture, char **errstr);

/* Set the current profile's picture. */
int
core_set_profile_picture (GdkPixbuf *picture, char **errstr);

/* Delete the current profile. */
int
core_delete_profile ();

/* Request the list of categories and execute the callback when done. */
void
core_request_categories (categories_request_callback callback);

/* Request the list of medias of a given category and execute the callback when
   done. */
void
core_request_medias (const char *category, medias_request_callback callback);

/* Request a media's poster image and execute the callback when done. */
void
core_request_poster (Media *m, picture_request_callback callback);

/* Search a media in the server. */
void
core_request_search (const char *category,
                     const char *search,
                     search_request_callback callback);

/* Free Core resources. */
void
core_destroy ();

#endif

