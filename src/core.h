/*
core.h - Application's core API headers.

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

#ifndef CORE_H
#define CORE_H

/*

// Core main data structure
typedef struct Core_s {
    char *server_address;
    char *profile;
} Core_t;

void
core_set_profile (const char *profile);

void
core_request_profile_picture (const char *profile,
                              picture_request_callback callback);

int
core_create_profile (const char *name, GdkPixbuf *picture, char **errstr);

int
core_set_profile_picture (GdkPixbuf *picture, char **errstr);

int
core_delete_profile ();

void
core_request_categories (categories_request_callback callback);

void
core_request_medias (const char *category, medias_request_callback callback);

void
core_request_poster (Media *m, picture_request_callback callback);

int
core_get_media_status (Media *m, MediaStatus *status);

int
core_download_media (Media *m, char **errstr);
*/

#include <string>

#include "profilepicturelistener.h"
#include "profileslistener.h"
#include "requestmanager.h"

class Core {

    private:

        // Server address
        std::string server_address;

        // Object to collect the finished requests
        RequestManager request_manager;

    public:

        Core (const std::string& server_address);
        ~Core ();

        // Request the list of profiles.
        void request_profiles (ProfilesListener& listener);

        // Request a profile's picture.
        void request_profile_picture (
            const std::string& profile, ProfilePictureListener& listener);

};

#endif

