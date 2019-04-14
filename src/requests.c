/*
requests.c - Server requests data types.

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

#include "requests.h"

ProfilesRequest *
request_profiles_new (profiles_request_callback callback)
{
    ProfilesRequest *r = g_new (ProfilesRequest, 1);
    r->profiles = g_ptr_array_new_with_free_func (g_free);
    r->callback = callback;
    return r;
}

void
request_profiles_destroy (ProfilesRequest *r)
{
    g_ptr_array_free (r->profiles, TRUE);
    g_free (r);
}

PictureRequest *
request_picture_new (GString *url,
                     void *data,
                     picture_request_callback callback)
{
    PictureRequest *r = g_new (PictureRequest, 1);
    r->data = data;
    r->url = url;
    r->picture = g_byte_array_new ();
    r->callback = callback;
    return r;
}

void
request_picture_destroy (PictureRequest *r)
{
    g_byte_array_free (r->picture, TRUE);
    g_string_free (r->url, TRUE);
    g_free (r);
}

CategoriesRequest *
request_categories_new (categories_request_callback callback)
{
    CategoriesRequest *r = g_new (CategoriesRequest, 1);
    r->categories = g_ptr_array_new_with_free_func (g_free);
    r->callback = callback;
    return r;
}

void
request_categories_destroy (CategoriesRequest *r)
{
    g_ptr_array_free (r->categories, TRUE);
    g_free (r);
}

MediasRequest *
request_medias_new (const char *category, medias_request_callback callback)
{
    MediasRequest *r = g_new (MediasRequest, 1);
    r->category = g_strdup (category);
    r->medias = g_ptr_array_new ();
    r->callback = callback;
    return r;
}

void
request_medias_add (MediasRequest *r, Media *m)
{
    g_ptr_array_add (r->medias, m);
}

void
request_medias_destroy (MediasRequest *r)
{
    g_ptr_array_free (r->medias, TRUE);
    g_free (r->category);
    g_free (r);
}

