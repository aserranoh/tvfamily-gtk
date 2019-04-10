/*
requests.h - Server requests data types.

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

#ifndef REQUESTS_H
#define REQUESTS_H

#include <glib.h>

#include "coretypes.h"

// Forward declarations
typedef struct ProfilesRequest_s ProfilesRequest;
typedef struct PictureRequest_s PictureRequest;
typedef struct CategoriesRequest_s CategoriesRequest;
typedef struct MediasRequest_s MediasRequest;

// Type of callback when the profiles request is done
typedef void (*profiles_request_callback)(ProfilesRequest *);

// Type of callback when a picture request is done
typedef void (*picture_request_callback)(PictureRequest *);

// Type of callback when the categories request is done
typedef void (*categories_request_callback)(CategoriesRequest *);

// Type of callback when the medias request is done
typedef void (*medias_request_callback)(MediasRequest *);

// Data to store when doing a request of the list of profiles
struct ProfilesRequest_s {
    GPtrArray *profiles;
    profiles_request_callback callback;
    int error;
};

struct PictureRequest_s {
    GString *url;
    char *id;
    GByteArray *picture;
    picture_request_callback callback;
    int error;
};

struct CategoriesRequest_s {
    GPtrArray *categories;
    categories_request_callback callback;
    int error;
};

struct MediasRequest_s {
    char *category;
    GArray *medias;
    medias_request_callback callback;
    int error;
};

/* Allocate a new ProfilesRequest struct. */
ProfilesRequest *
request_profiles_new (profiles_request_callback callback);

#define request_profiles_size(r)    (r->profiles->len)

#define request_profiles_get(r, i)  (g_ptr_array_index (r->profiles, i))

/* Free a ProfilesRequest instance. */
void
request_profiles_destroy (ProfilesRequest *r);

/* Allocate a new PictureRequest struct. */
PictureRequest *
request_picture_new (const char *id,
                     GString *url,
                     picture_request_callback callback);

#define request_picture_size(r) (r->picture->len)

/* Free a PictureRequest instance. */
void
request_picture_destroy (PictureRequest *r);

/* Allocate a new CategoriesRequest struct. */
CategoriesRequest *
request_categories_new (categories_request_callback callback);

#define request_categories_size(r)      (r->categories->len)

#define request_categories_get(r, i)    (g_ptr_array_index (r->categories, i))

/* Free a CategoriesRequest instance. */
void
request_categories_destroy (CategoriesRequest *r);

/* Allocate a new ProfilesRequest struct. */
MediasRequest *
request_medias_new (const char *category, medias_request_callback callback);

/* Set the number of medias that the request will hold. */
void
request_medias_set_size (MediasRequest *r, size_t size);

/* Add a media to the request. */
void
request_medias_add (MediasRequest *r, Media *m);

#define request_medias_size(r)      (r->medias->len)

#define request_medias_get(r, i)    (&g_array_index (r->medias, Media, i))

/* Free a MediasRequest instance. */
void
request_medias_destroy (MediasRequest *r);

#endif

