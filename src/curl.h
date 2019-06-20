/*
curl.h - Wrapper to the cURL functions.

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

#ifndef CURL_H
#define CURL_H

#include <curl/curl.h>
#include <stdexcept>
#include <string>

class Curl {

    private:

        friend class CurlGlobal;

        struct CurlGlobal {

            CurlGlobal () {
                // Initialize curl. If something go wrong, abort.
                if (curl_global_init (CURL_GLOBAL_ALL)) {
                    throw std::runtime_error ("error in curl_global_init");
                }
            }

            ~CurlGlobal () {
                // Free curl resources
                curl_global_cleanup ();
            }
        };

        // Global initialization
        static CurlGlobal curl_global;

        // CURL easy handler
        CURL* handler;

    public:

        Curl ();
        ~Curl ();

        // Wrappers to curl_easy_setopt
        void setopt (CURLoption option, const std::string& s);
        void setopt (CURLoption option, void* ptr);
        void setopt (
            CURLoption option, size_t (*func)(void*, size_t, size_t, void*));
        void setopt (CURLoption option, int i);

        // Wrapper to curl_easy_perform
        void perform ();

};

#endif

