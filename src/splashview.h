/*
splashview.h - The view with the TVFamily logo.

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

#ifndef SPLASHVIEW_H
#define SPLASHVIEW_H

#include <gtkmm/image.h>
#include <gtkmm/window.h>

#include "view.h"

class SplashView: public View {

    private:

        // CONSTANTS

        static const float LOGO_RATIO;
        static const int TIMEOUT;

        // Image with the logo
        Gtk::Image logo;

        // Stores whether the timeout signal has been connected or not
        bool timeout_signal_connected;

    public:

        SplashView (ViewControllerInterface& controller);
        ~SplashView ();

        // Show this view
        void show ();

    private:

        // Timeout to change view consumed
        bool on_timeout ();

};

#endif

