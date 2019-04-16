/*
core.h - Application's core API implementation.

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
#include <jansson.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>

#include "core.h"

// GLOBAL VARIABLES

// The Core main data structure
Core core;

// PRIVATE FUNCTIONS

/* Function that copies data received from the server to a buffer. */
static size_t
core_curl_receive (void *buffer, size_t size, size_t nmemb, void *userp)
{
    g_byte_array_append ((GByteArray *)userp, buffer, nmemb);
    return nmemb;
}

/* Executes a GET or POST request in the server. Return 0 if OK or -1 on error.
   On exit, if buf contains the read data.
*/
static int
core_http_request (GString *url,
                   const char *data,
                   size_t size,
                   GByteArray *buf)
{
    int err = -1;
    CURL *h;
    CURLcode c;
    curl_mime *multipart = NULL;

    // Make the request
    if (!(h = curl_easy_init ())) {
        warnx ("core_http_request: error in curl_easy_init");
        goto cleanup;
    }
    if ((c = curl_easy_setopt (h, CURLOPT_URL, url->str)) != CURLE_OK) {
        warnx ("core_http_request: error in curl_easy_setop (URL): %s",
            curl_easy_strerror (c));
        goto cleanup;
    }
    if (data) {
        multipart = curl_mime_init (h);
        curl_mimepart *part = curl_mime_addpart(multipart);
        curl_mime_name(part, "file");
        curl_mime_filename(part, "profile.png");
        curl_mime_type(part, "image/png");
        curl_mime_data(part, data, size);
        curl_easy_setopt(h, CURLOPT_MIMEPOST, multipart);
    }
    c = curl_easy_setopt (h, CURLOPT_WRITEFUNCTION, core_curl_receive);
    if (c != CURLE_OK) {
        warnx ("core_http_request: error in curl_easy_setop "
            "(WRITEFUNCTION): %s", curl_easy_strerror (c));
        goto cleanup;
    }
    c = curl_easy_setopt (h, CURLOPT_WRITEDATA, buf);
    if (c != CURLE_OK) {
        warnx ("core_http_request: error in curl_easy_setop (WRITEDATA): %s",
            curl_easy_strerror (c));
        goto cleanup;
    }
    c = curl_easy_setopt (h, CURLOPT_FAILONERROR, 1);
    if (c != CURLE_OK) {
        warnx ("core_http_request: error in curl_easy_setop (FAILONERROR): %s",
            curl_easy_strerror (c));
        goto cleanup;
    }
    if ((c = curl_easy_perform (h)) != CURLE_OK) {
        warnx ("core_http_request: error in curl_easy_perform: %s",
            curl_easy_strerror (c));
        goto cleanup;
    }
    err = 0;
cleanup:
    if (h) curl_easy_cleanup (h);
    if (multipart) curl_mime_free (multipart);
    return err;
}

/* Executes a GET or POST request in the server. Return 0 if OK or -1 on error.
   On exit, if not NULL ret contains the json node with the result.
*/
static int
core_http_json (GString *url, const char *data, size_t size, json_t **ret)
{
    int err = -1;
    GByteArray *buf = g_byte_array_new ();
    json_error_t jerr;
    int free_json = 1;

    // Make the request
    *ret = NULL;
    if (core_http_request (url, data, size, buf)) {
        goto cleanup;
    }

    // Load the json data and check the error code.
    if (!(*ret = json_loadb (buf->data, buf->len, 0, &jerr))) {
        warnx ("core_http_json: error in json_loads (line %d): %s\n",
            jerr.line, jerr.text);
        goto cleanup;
    }
    if (!json_is_object (*ret)) {
        warnx ("core_http_json: returned json is not an object");
        goto cleanup;
    }
    json_t *retcode = json_object_get (*ret, "code");
    if (!json_is_integer (retcode)) {
        warnx ("core_http_json: code attribute is not integer");
        goto cleanup;
    }
    if (json_integer_value (retcode)) {
        json_t *retstrerr = json_object_get (*ret, "error");
        if (!json_is_string (retstrerr)) {
            warnx ("core_http_json: cannot decode error attribute");
        } else {
            warnx ("core_http_json: get returned with error: %s",
                json_string_value (retstrerr));
            free_json = 0;
        }
    } else {
        err = 0;
        free_json = 0;
    }
cleanup:
    if (free_json && *ret) {
        json_decref (*ret);
        *ret = NULL;
    }
    g_byte_array_free (buf, TRUE);
    return err;
}

/* Thread to get the list of profiles from the server. */
static void *
core_request_profiles_thread (void *request)
{
    ProfilesRequest *r = (ProfilesRequest *)request;
    json_t *j, *profiles, *profile;
    int i;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (url, "%s/api/getprofiles", core.server_address);

    // Make the request
    r->error = -1;
    if (!core_http_json (url, NULL, 0, &j)) {
        // Extract the data from the received JSON element
        profiles = json_object_get (j, "profiles");
        if (!json_is_array (profiles)) {
            warnx ("core_request_profiles_thread: profiles not an array");
        } else {
            int len = json_array_size (profiles);
            for (i = 0; i < len; i++) {
                profile = json_array_get (profiles, i);
                if (!json_is_string (profile)) {
                    warnx ("core_request_profiles_thread: "
                        "profile not a string");
                    break;
                } else {
                    char *s = g_strdup (json_string_value (profile));
                    g_ptr_array_add (r->profiles, s);
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
}

/* Thread to get a picture from the server. */
static void *
core_request_picture_thread (void *request)
{
    PictureRequest *r = (PictureRequest *)request;

    // Make the request
    r->error = core_http_request (r->url, NULL, 0, r->picture);
    r->callback (r);
}

/* Thread to get the list of categories from the server. */
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
}

/* Thread to get the list of medias from the server. */
static void *
core_request_medias_thread (void *request)
{
    MediasRequest *r = (MediasRequest *)request;
    json_t *j, *jmedias, *jmedia;
    Media *m;
    int i;

    // Build the URL
    GString *url = g_string_new (NULL);
    g_string_printf (
        url, "%s/api/gettop?profile=", core.server_address);
    g_string_append_uri_escaped (url, core.profile, NULL, TRUE);
    g_string_append (url, "&category=");
    g_string_append_uri_escaped (url, r->category, NULL, TRUE);

    // Make the request
    r->error = -1;
    if (!core_http_json (url, NULL, 0, &j)) {
        // Extract the data from the received JSON element
        jmedias = json_object_get (j, "top");
        if (!json_is_array (jmedias)) {
            warnx ("core_request_medias_thread: medias not an array");
        } else {
            int len = json_array_size (jmedias);
            for (i = 0; i < len; i++) {
                jmedia = json_array_get (jmedias, i);
                if (!json_is_object (jmedia)) {
                    warnx ("core_request_medias_thread: media not an object");
                    break;
                } else {
                    if ((m = media_new (jmedia))) {
                        request_medias_add (r, m);
                    } else {
                        warnx ("core_request_medias_thread: cannot create "
                            "media from json");
                    }
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
}

// PUBLIC FUNCTIONS

void
core_create(const char *server_address)
{
    // Initialize data members
    core.server_address = g_strdup (server_address);
    core.profile = NULL;

    // Initialize curl. If something go wrong, abort.
    if (curl_global_init (CURL_GLOBAL_ALL)) {
        errx (1, "core_create: error in curl_global_init");
    }
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
core_request_profiles (profiles_request_callback callback)
{
    pthread_t t;
    ProfilesRequest *r = request_profiles_new (callback);

    // Launch the thread to receive the data from the server
    if (pthread_create (&t, NULL, core_request_profiles_thread, r)) {
        err (1, "core_request_profiles: error in pthread_create");
    }
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

void
core_destroy ()
{
    // Free curl resources
    curl_global_cleanup ();
}

