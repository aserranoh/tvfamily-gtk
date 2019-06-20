/*
splashview.cpp - The view with the TVFamily logo.

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

#include <glibmm/main.h>
#include <gdkmm/screen.h>

#include "paths.h"
#include "splashview.h"

// Initialization of constant values
const float SplashView::LOGO_RATIO = 0.66;
//const int SplashView::TIMEOUT = 3000;
const int SplashView::TIMEOUT = 0;

SplashView::SplashView (ViewControllerInterface& controller):
    View (controller), logo (), timeout_signal_connected (false)
{
    // Add the logo to the main box
    auto& box = get_box ();
    box.pack_start (logo, true, false);

    // Load the logo into a pixbuf
    auto screen = Gdk::Screen::get_default ();
    auto pixbuf = Gdk::Pixbuf::create_from_file (
        Paths::get_logo (), screen->get_width () * LOGO_RATIO, -1);

    // Set the pixbuf of the logo image
    logo.set (pixbuf);

    // Show all elements
    box.show_all ();
}

SplashView::~SplashView ()
{}

void SplashView::show ()
{
    // Change of view after some seconds
    if (not timeout_signal_connected) {
        Glib::signal_timeout ().connect (
            sigc::mem_fun (*this, &SplashView::on_timeout), TIMEOUT);
        timeout_signal_connected = true;
    }
}

bool SplashView::on_timeout ()
{
    get_controller ().switch_view ("choose-profile");
    get_box ().remove (logo);
    return false;
}

