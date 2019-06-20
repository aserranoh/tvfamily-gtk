/*
request.cpp - Base class for all the requests.

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

#include "curl.h"
#include "request.h"

#include <iostream>

Request::Request (const std::string& server_address):
    server_address (server_address),
    request_thread (&Request::run_thread, this)
{}

Request::~Request ()
{}

/*
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
    c = curl_easy_setopt (h, CURLOPT_FOLLOWLOCATION, 1);
    if (c != CURLE_OK) {
        warnx ("core_http_request: error in curl_easy_setop "
            "(FOLLOWLOCATION): %s", curl_easy_strerror (c));
        goto cleanup;
    }
    if ((c = curl_easy_perform (h)) != CURLE_OK) {
        warnx ("core_http_request: error in curl_easy_perform: %s",
            curl_easy_strerror (c));
        goto cleanup;
    }
    err = 0;
cleanup:
    //if (h) curl_easy_cleanup (h);
    if (multipart) curl_mime_free (multipart);
    return err;
}
*/

Glib::RefPtr<Glib::ByteArray> Request::get_request (
    const std::string& api_function)
{
    Curl curl;
    auto url = server_address + "/api/" + api_function;
    curl.setopt (CURLOPT_URL, url);
    curl.setopt (CURLOPT_WRITEFUNCTION, &Request::receive);
    auto buffer = Glib::ByteArray::create ();
    curl.setopt (CURLOPT_WRITEDATA, &buffer);
    curl.setopt (CURLOPT_FAILONERROR, 1);
    curl.setopt (CURLOPT_FOLLOWLOCATION, 1);
    curl.perform ();
    return buffer;
}

void Request::get_json_request (
    const std::string& api_function, rapidjson::Document& document)
{
    auto data = get_request (api_function);
    // Force a null character at the end
    const guint8 end[] = {0};
    data->append (end, 1);
    document.Parse (reinterpret_cast<const char*>(data->get_data ()));
    // Check that the returned string is indeed a valid JSON obect
    if (not document.IsObject ()) {
        throw std::runtime_error (
            "request " + api_function + " returned invalid JSON string");
    } else {
        // Check the return code
        if (not document.HasMember ("code")) {
            throw std::runtime_error (
                "request " + api_function + " returned invalid JSON string");
        } else {
            auto code = document["code"].GetInt ();
            if (code) {
                throw std::runtime_error ("request " + api_function
                    + " returned with code " + std::to_string (code));
            }
        }
    }
}

void Request::run_thread ()
{
    run ();
}

size_t Request::receive (void* buffer, size_t size, size_t nmemb, void* userp)
{
    auto byte_array = static_cast<Glib::RefPtr<Glib::ByteArray>*>(userp);
    (*byte_array)->append (static_cast<const guint8*>(buffer), nmemb);
    return nmemb;
}

