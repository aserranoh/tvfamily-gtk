/*
paths.h - Paths for different elements of the application.

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

#ifndef PATHS_H
#define PATHS_H

#include <filesystem>

/*
const char *
paths_get_default_picture ();*/

class Paths {

    private:

        static const std::filesystem::path static_path;
        static const std::filesystem::path logo_file;
        static const std::filesystem::path logo_path;
        static const std::filesystem::path styles_file;
        static const std::filesystem::path styles_path;

    public:

        // Return an image file.
        static std::filesystem::path get_image (const std::string& image);

        // Return the path to the logo image file.
        static const std::filesystem::path& get_logo ();

        // Return the path to the styles CSS file.
        static const std::filesystem::path& get_styles ();

};

#endif

