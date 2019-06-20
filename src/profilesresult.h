/*
profilesresult.h - The result from a profiles request.

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

#ifndef PROFILESRESULT_H
#define PROFILESRESULT_H

#include <vector>
#include <string>

#include "requestresult.h"

class ProfilesResult: public RequestResult {

    private:

        // List of profiles
        std::vector<std::string> profiles;

    public:

        ProfilesResult ();
        ~ProfilesResult ();

        // Add a profile.
        inline void add (const std::string& profile)
            { profiles.push_back (profile); }

        // Return the list of profiles.
        inline std::vector<std::string>& get_profiles () { return profiles; }

        // Return the number of profiles.
        inline int size () const { return profiles.size (); }

};

#endif

