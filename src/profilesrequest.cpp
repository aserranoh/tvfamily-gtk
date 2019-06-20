/*
profilesrequest.cpp - Request the list of profiles.

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

#include <iostream>
#include <rapidjson/document.h>

#include "profilesrequest.h"
#include "profilesresult.h"

ProfilesRequest::ProfilesRequest (
    const std::string& server_address, ProfilesListener& listener):
        Request (server_address), listener(listener)
{}

ProfilesRequest::~ProfilesRequest ()
{}

void ProfilesRequest::run ()
{
    auto r = std::make_unique<ProfilesResult>();
    rapidjson::Document d;

    try {
        get_json_request ("getprofiles", d);
        if (not d.HasMember ("profiles")) {
            std::cerr << "getprofiles request: no 'profiles' member in json"
                << std::endl;
            r->set_error (true);
        } else {
            const rapidjson::Value& profiles_array = d["profiles"];
            for (rapidjson::SizeType i = 0; i < profiles_array.Size (); i++) {
                r->add (profiles_array[i].GetString ());
            }
            r->set_error (false);
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what () << std::endl;
        r->set_error (true);
    }
    listener.profiles_received (r);
}

