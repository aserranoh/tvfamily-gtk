/*
paths.c - Paths for different elements of the application.

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

#include "paths.h"

/*#define DEFAULT_PICTURE_FILE    "profile-default.svg"

static const char *default_picture_path = STATIC_PATH "/" DEFAULT_PICTURE_FILE;

const char *
paths_get_default_picture ()
{
    return default_picture_path;
}*/

const std::filesystem::path Paths::static_path ("data");
const std::filesystem::path Paths::logo_file ("tvfamily.svg");
const std::filesystem::path Paths::styles_file ("styles.css");
const std::filesystem::path Paths::logo_path (static_path / logo_file);
const std::filesystem::path Paths::styles_path (static_path / styles_file);

std::filesystem::path Paths::get_image (const std::string& image)
{
    return static_path / (image + ".svg");
}

const std::filesystem::path& Paths::get_logo ()
{
    return logo_path;
}

const std::filesystem::path& Paths::get_styles ()
{
    return styles_path;
}

