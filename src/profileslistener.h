/*
profileslistener.h - Interface to receive the profiles request results.

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

#ifndef PROFILESLISTENER_H
#define PROFILESLISTENER_H

#include <memory>

#include "profilesresult.h"

class ProfilesListener {

    public:

        // Notify that the list of profiles is ready
        virtual void profiles_received (
            std::unique_ptr<ProfilesResult>& result) = 0;

};

#endif

