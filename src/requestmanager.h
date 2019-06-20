/*
requestmanager.h - Container for the current requests.

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

#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <list>
#include <mutex>
#include <thread>

#include "request.h"

class RequestManager {

    private:

        // The list of requests
        std::list<std::unique_ptr<Request> > requests;

        // Thread that collects other threads
        std::thread collector_thread;

        // Order to stop the collector thread
        bool stop;

        // Mutex to protect the list of requests
        std::mutex requests_mutex;

    public:

        RequestManager ();
        ~RequestManager ();

        // Add a request.
        void add (std::unique_ptr<Request>& request);

    private:

        // Request collector function
        void run ();

};

#endif

