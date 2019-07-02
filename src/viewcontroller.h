/*
viewcontroller.h - The controller for the graphic layer.

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

#ifndef VIEWCONTROLLER_H
#define VIEWCONTROLLER_H

#include <gtkmm/application.h>
#include <gtkmm/stack.h>
#include <gtkmm/window.h>
#include <map>
#include <string>

#include "core.h"
#include "mediainfoview.h"
#include "mediasview.h"
#include "newprofileview.h"
#include "pictureview.h"
#include "playerview.h"
#include "profilesview.h"
#include "splashview.h"
#include "viewcontrollerinterface.h"

class ViewController: public ViewControllerInterface {

    private:

        // Constants
        static const int FULLSCREEN_KEY = GDK_KEY_F11;

        // The GTK application
        Glib::RefPtr<Gtk::Application> app;

        // The GTK window
        Gtk::Window window;

        // Container for all the views
        Gtk::Stack stack;

        // Dictionary with the views indexed by name
        std::map<std::string, View*> views_map;

        // ID of the last view
        std::string last_view;

        // Flag that stores if the window is in fullscreen mode
        bool is_fullscreen;

        // References to the views
        SplashView     splash_view;
        ProfilesView   profiles_view;
        NewProfileView newprofile_view;
        MediasView     medias_view;
        PictureView    picture_view;
        MediaInfoView  mediainfo_view;
        PlayerView     player_view;

        // The application's core
        Core core;

    public:

        ViewController (Glib::RefPtr<Gtk::Application>& app,
                        const std::string& server_address);
        ~ViewController ();

        // Implementation of ViewControllerInterface interface
        inline Gtk::Window& get_window () { return window; }

        // Implementation of ViewControllerInterface interface
        inline Core& get_core () { return core; }

        // Implementation of ViewControllerInterface interface
        void switch_view (const std::string& new_view);

        // Implementation of ViewControllerInterface interface
        void switch_view (const std::string& new_view,
                          const ViewSwitchData& data);

        // Implementation of ViewControllerInterface interface
        void back ();

        // Implementation of ViewControllerInterface interface
        Gtk::Widget* get_current_view ();

        // Implementation of ViewControllerInterface interface
        void exit ();

    private:

        // Callback executed when a key is pressed (to manage fullscreen)
        bool on_key_press (GdkEventKey* event);

        /* Callback executed when the window state is changed (to update
           fullscreen mode. */
        bool on_window_state (GdkEventWindowState* event);

};

#endif

