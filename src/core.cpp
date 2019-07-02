/*
core.cpp - Application's core API implementation.

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

/*#include <err.h>
#include <jansson.h>
#include <stdarg.h>
#include <string.h>

// PRIVATE FUNCTIONS

static void *
core_request_picture_thread (void *request)
{
    PictureRequest *r = (PictureRequest *)request;

    // Make the request
    r->error = core_http_request (r->url, NULL, 0, r->picture);
    r->callback (r);
    return nullptr;
}

static void *
core_request_categories_thread (void *request)
{
    CategoriesRequest *r = (CategoriesRequest *)request;
    json_t *j, *categories, *category;
    int i;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (url, "%s/api/getcategories", core.server_address);

    // Make the request
    r->error = -1;
    if (!core_http_json (url, NULL, 0, &j)) {
        // Extract the data from the received JSON element
        categories = json_object_get (j, "categories");
        if (!json_is_array (categories)) {
            warnx ("core_request_categories_thread: categories not an array");
        } else {
            int len = json_array_size (categories);
            for (i = 0; i < len; i++) {
                category = json_array_get (categories, i);
                if (!json_is_string (category)) {
                    warnx ("core_request_categories_thread: "
                        "category not a string");
                    break;
                } else {
                    char *s = g_strdup (json_string_value (category));
                    g_ptr_array_add (r->categories, s);
                }
            }
            if (i == len) {
                r->error = 0;
            }
        }
    }
    if (j) {
        json_decref (j);
    }
    g_string_free (url, TRUE);
    r->callback (r);
    return nullptr;
}

static int
core_get_medias_list (GString *url, const char *root, GPtrArray *a)
{
    json_t *j, *jmedias, *jmedia;
    int i, error = -1;
    Media *m;

    // Make the request
    if (!core_http_json (url, NULL, 0, &j)) {
        // Extract the data from the received JSON element
        jmedias = json_object_get (j, root);
        if (!json_is_array (jmedias)) {
            warnx ("core_get_medias_list: medias not an array");
        } else {
            int len = json_array_size (jmedias);
            for (i = 0; i < len; i++) {
                jmedia = json_array_get (jmedias, i);
                if (!json_is_object (jmedia)) {
                    warnx ("core_get_medias_list: media not an object");
                    break;
                } else {
                    if ((m = media_new (jmedia))) {
                        g_ptr_array_add (a, m);
                    } else {
                        warnx ("core_get_medias_list: cannot create media from"
                            " json");
                    }
                }
            }
            if (i == len) {
                error = 0;
            }
        }
    }
    if (j) {
        json_decref (j);
    }
    g_string_free (url, TRUE);
    return error;
}

static void *
core_request_medias_thread (void *request)
{
    MediasRequest *r = (MediasRequest *)request;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/gettop?profile=", core.server_address);
    g_string_append_uri_escaped (url, core.profile, NULL, TRUE);
    g_string_append (url, "&category=");
    g_string_append_uri_escaped (url, r->category, NULL, TRUE);

    // Make the request
    r->error = core_get_medias_list (url, "top", r->medias);
    r->callback (r);
    return nullptr;
}

static void *
core_request_search_thread (void *request)
{
    SearchRequest *r = (SearchRequest *)request;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/search?category=", core.server_address);
    g_string_append_uri_escaped (url, r->category, NULL, TRUE);
    g_string_append (url, "&text=");
    g_string_append_uri_escaped (url, r->search, NULL, TRUE);

    // Make the request
    r->error = core_get_medias_list (url, "search", r->result);
    r->callback (r);
    return nullptr;
}

// PUBLIC FUNCTIONS

void
core_create(const char *server_address)
{
    // Initialize data members
    core.server_address = g_strdup (server_address);
    core.profile = NULL;
}

void
core_set_profile (const char *profile)
{
    if (core.profile) {
        g_free (core.profile);
    }
    core.profile = g_strdup (profile);
}

void
core_request_profile_picture (const char *profile,
                              picture_request_callback callback)
{
    pthread_t t;
    
    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/getprofilepicture?name=", core.server_address);
    g_string_append_uri_escaped (url, profile, NULL, TRUE);

    // Create the request object
    PictureRequest *r = request_picture_new (
        url, g_strdup (profile), callback);

    // Launch the thread to receive the data from the server
    if (pthread_create (&t, NULL, core_request_picture_thread, r)) {
        err (1, "core_request_profile_picture: error in pthread_create");
    }
}

int
core_create_profile (const char *name, GdkPixbuf *picture, char **errstr)
{
    json_t *j;
    int err = -1;
    char *pic = NULL;
    size_t picsize = 0;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (url, "%s/api/createprofile?name=", core.server_address);
    g_string_append_uri_escaped (url, name, NULL, TRUE);

    // Save the pixbuf to a buffer, if any
    if (picture) {
        gdk_pixbuf_save_to_buffer (picture, &pic, &picsize, "png", NULL, NULL);
    }

    // Make the request
    if (!core_http_json (url, pic, picsize, &j)) {
        // If core_get_json returns 0, we're done
        err = 0;
    } else {
        // Fill the error message
        json_t *retstrerr = json_object_get (j, "error");
        *errstr = g_strdup (json_string_value (retstrerr));
    }

    // Cleanup
    if (pic) g_free (pic);
    if (j) json_decref (j);
    g_string_free (url, TRUE);
    return err;
}

int
core_set_profile_picture (GdkPixbuf *picture, char **errstr)
{
    json_t *j;
    int err = -1;
    char *pic = NULL;
    size_t picsize = 0;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/setprofilepicture?name=", core.server_address);
    g_string_append_uri_escaped (url, core.profile, NULL, TRUE);

    // Save the pixbuf to a buffer, if any
    if (picture) {
        gdk_pixbuf_save_to_buffer (picture, &pic, &picsize, "png", NULL, NULL);
    }

    // Make the request
    if (!core_http_json (url, pic, picsize, &j)) {
        // If core_get_json returns 0, we're done
        err = 0;
    } else {
        // Fill the error message
        json_t *retstrerr = json_object_get (j, "error");
        *errstr = g_strdup (json_string_value (retstrerr));
    }

    // Cleanup
    if (pic) g_free (pic);
    if (j) json_decref (j);
    g_string_free (url, TRUE);
    return err;
}

int
core_delete_profile ()
{
    json_t *j;
    int err = -1;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (url, "%s/api/deleteprofile?name=", core.server_address);
    g_string_append_uri_escaped (url, core.profile, NULL, TRUE);

    // Make the request
    if (!core_http_json (url, NULL, 0, &j)) {
        // If core_get_json returns 0, we're done
        err = 0;
    }

    // Cleanup
    if (j) json_decref (j);
    g_string_free (url, TRUE);
    return err;
}

void
core_request_categories (categories_request_callback callback)
{
    pthread_t t;
    CategoriesRequest *r = request_categories_new (callback);

    // Launch the thread to receive the data from the server
    if (pthread_create (&t, NULL, core_request_categories_thread, r)) {
        err (1, "core_request_categories: error in pthread_create");
    }
}

void
core_request_medias (const char *category, medias_request_callback callback)
{
    pthread_t t;
    MediasRequest *r = request_medias_new (category, callback);

    // Launch the thread to receive the data from the server
    if (pthread_create (&t, NULL, core_request_medias_thread, r)) {
        err (1, "core_request_medias: error in pthread_create");
    }
}

void
core_request_poster (Media *m, picture_request_callback callback)
{
    pthread_t t;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/getposter?id=", core.server_address);
    g_string_append_uri_escaped (url, m->title_id, NULL, TRUE);

    // Build the request
    PictureRequest *r = request_picture_new (url, m, callback);

    // Launch the thread to receive the data from the server
    if (pthread_create (&t, NULL, core_request_picture_thread, r)) {
        err (1, "core_request_poster: error in pthread_create");
    }
}

}

int
core_get_media_status (Media *m, MediaStatus *status)
{
    int err = 1;
    json_t *j, *jstatus, *j2;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/getmediastatus?id=", core.server_address);
    g_string_append_uri_escaped (url, m->title_id, NULL, TRUE);
    if (m->season > 0 && m->episode > 0) {
        g_string_append_printf (
            url, "&season=%d&episode=%d", m->season, m->episode);
    }

    // Make the request
    if (!core_http_json (url, NULL, 0, &j)) {
        // Extract the data from the received JSON element
        jstatus = json_object_get (j, "status");
        if (!json_is_object (jstatus)) {
            warnx ("core_get_media_status: status not an object");
        } else {
            j2 = json_object_get (jstatus, "status");
            if (!json_is_integer (j2)) {
                warnx ("core_get_media_status: status is not integer");
            } else {
                status->status = json_integer_value (j2);
                j2 = json_object_get (jstatus, "message");
                if (!json_is_string (j2)) {
                    warnx ("core_get_media_status: message is not string");
                } else {
                    status->message = g_strdup (json_string_value (j2));
                    j2 = json_object_get (jstatus, "progress");
                    if (!json_is_integer (j2)) {
                        warnx (
                            "core_get_media_status: progress is not integer");
                    } else {
                        status->progress = json_integer_value (j2);
                        if (status->progress < 0) {
                            status->progress = 0;
                        } else if (status->progress > 100) {
                            status->progress = 100;
                        }
                        err = 0;
                    }
                }
            }
        }
    }
    if (j) {
        json_decref (j);
    }
    g_string_free (url, TRUE);
    return err;
}

int
core_download_media (Media *m, char **errstr)
{
    int err = 1;
    json_t *j, *retstrerr;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/download?profile=", core.server_address);
    g_string_append_uri_escaped (url, core.profile, NULL, TRUE);
    g_string_append (url, "&id=");
    g_string_append_uri_escaped (url, m->title_id, NULL, TRUE);
    if (m->season > 0 && m->episode > 0) {
        g_string_append_printf (
            url, "&season=%d&episode=%d", m->season, m->episode);
    }

    // Make the request
    if (!core_http_json (url, NULL, 0, &j)) {
        // If core_get_json returns 0, we're done
        err = 0;
    } else {
        // Fill the error message
        json_t *retstrerr = json_object_get (j, "error");
        *errstr = g_strdup (json_string_value (retstrerr));
    }

    // Cleanup
    if (j) json_decref (j);
    g_string_free (url, TRUE);
    return err;
}

*/

#include "core.h"
#include "profilesrequest.h"

Core::Core (const std::string& server_address):
    server_address (server_address), request_manager ()
{}

Core::~Core ()
{}

void Core::request_profiles (ProfilesListener& listener)
{
    std::unique_ptr<Request> request = std::make_unique<ProfilesRequest> (
        server_address, listener);
    request_manager.add (request);
}

void Core::request_profile_picture (
    const std::string& profile, ProfilePictureListener& listener)
{
    
}

