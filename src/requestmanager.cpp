/*
requestmanager.cpp - Container for the current requests.

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

#include <chrono>

#include "requestmanager.h"

#include <iostream>

using namespace std::chrono_literals;

RequestManager::RequestManager ():
    requests (), stop (false), collector_thread (&RequestManager::run, this)
{}

RequestManager::~RequestManager ()
{
    stop = true;
    collector_thread.join ();
}

void RequestManager::add (std::unique_ptr <Request>& request)
{
    std::lock_guard<std::mutex> lock(requests_mutex);
    requests.push_back (std::move (request));
}

void RequestManager::run ()
{
    while (not stop) {
        if (requests.size ()) {
            std::unique_ptr<Request> request;
            {
                std::lock_guard<std::mutex> lock(requests_mutex);
                request = std::move (requests.front ());
                requests.pop_front ();
            }
            request->get_thread ().join ();
            std::cout << "request joined" << std::endl;
        } else {
            std::this_thread::sleep_for (2s);
        }
    }
}

