/*
curl.cpp - Wrapper to the cURL functions.

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

Curl::CurlGlobal Curl::curl_global;

Curl::Curl ():
    handler (nullptr)
{
    if (!(handler = curl_easy_init ())) {
        throw std::runtime_error ("error in curl_easy_init");
    }
}

Curl::~Curl ()
{
    if (handler) {
        curl_easy_cleanup (handler);
    }
}

void Curl::setopt (CURLoption option, const std::string& s)
{
    CURLcode c;
    if ((c = curl_easy_setopt (handler, option, s.c_str())) != CURLE_OK) {
        throw std::runtime_error ("error in curl_easy_setop ("
            + std::to_string(option) + "): " + curl_easy_strerror (c));
    }
}

void Curl::setopt (CURLoption option, void* ptr)
{
    CURLcode c;
    if ((c = curl_easy_setopt (handler, option, ptr)) != CURLE_OK) {
        throw std::runtime_error ("error in curl_easy_setop ("
            + std::to_string(option) + "): " + curl_easy_strerror (c));
    }
}

void Curl::setopt (
    CURLoption option, size_t (*func)(void*, size_t, size_t, void*))
{
    CURLcode c;
    if ((c = curl_easy_setopt (handler, option, func)) != CURLE_OK) {
        throw std::runtime_error ("error in curl_easy_setop ("
            + std::to_string(option) + "): " + curl_easy_strerror (c));
    }
}

void Curl::setopt (CURLoption option, int i)
{
    CURLcode c;
    if ((c = curl_easy_setopt (handler, option, i)) != CURLE_OK) {
        throw std::runtime_error ("error in curl_easy_setop ("
            + std::to_string(option) + "): " + curl_easy_strerror (c));
    }
}

void Curl::perform ()
{
    CURLcode c;
    if ((c = curl_easy_perform (handler)) != CURLE_OK) {
        throw std::runtime_error (std::string("error in curl_easy_perform: ")
            + curl_easy_strerror (c));
    }
}

