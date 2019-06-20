/*
profilesbox.h - Holds a list of ProfileButton.

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

#ifndef PROFILESBOX_H
#define PROFILESBOX_H

#include <string>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>

class ProfilesBox {

    private:

        // Profile button size
        int size;

        // Profile clicked callback
        sigc::slot<void> callback;

        // Box to scroll the profiles buttons
        Gtk::ScrolledWindow box;

        // Box inside the scrolled window
        Gtk::Box profile_buttons_box;

    public:

        ProfilesBox (int size, sigc::slot<void> callback);
        ~ProfilesBox ();

        // Return the GTK box that contains the controls
        inline Gtk::ScrolledWindow& get_box ()
            { return box; }

        // Set the list of profiles
        void set (const std::vector<std::string>& profiles);

        /* Give the focus to a given profile.
           Return true if any profile got the focus. */
        bool set_focus (int index);

    private:

        // This list of profiles is shown.
        void on_show ();

};

#endif

