/*
request.h - Base class for all the requests.

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

#ifndef REQUEST_H
#define REQUEST_H

#include <glibmm/bytearray.h>
#include <rapidjson/document.h>
#include <string>
#include <thread>

class Request {

    private:

        // Thread that runs this request.
        std::thread request_thread;

        // Server address
        std::string server_address;

    public:

        Request (const std::string& server_address);
        virtual ~Request ();

        // Return the thread that runs this request.
        inline std::thread& get_thread () { return request_thread; }

        // Run this request.
        virtual void run () = 0;

    protected:

        // Make an HTTP request
        Glib::RefPtr<Glib::ByteArray> get_request (
            const std::string& api_function);

        // Make an HTTP request and extract the returned JSON
        void get_json_request (
            const std::string& api_function, rapidjson::Document& document);

    private:

        // Bootstrap of the thread function
        void run_thread ();

        // Function to receive data from the HTTP request
        static size_t receive (
            void* buffer, size_t size, size_t nmemb, void* userp);

};

#endif

