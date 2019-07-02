/*
profilebutton.h - A button with a profile picture and name.

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

#ifndef PROFILEBUTTON_H
#define PROFILEBUTTON_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <string>

#include "profilebuttonlistener.h"

class ProfileButton {

    private:

        // The listener to receive the events from this button
        ProfileButtonListener& listener;

        // Box that contains the button and the label.
        Gtk::Box box;

        // The GTK button.
        Gtk::Button button;

        // The image that goes into the button.
        Gtk::Image image;

        // The label that contains the name of the profile.
        Gtk::Label label;

    public:

        ProfileButton (const std::string& name,
                       int size,
                       ProfileButtonListener& listener);
        ~ProfileButton ();

        // Return the profile's name.
        inline const std::string get_name () const
            { return label.get_text (); }

        // Return the GTK button
        inline Gtk::Button& get_button () { return button; }

    private:

        // The button has been clicked.
        void on_button_clicked ();

};

#endif

