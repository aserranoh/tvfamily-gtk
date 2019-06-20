/*
animatedbutton.h - A button with two images that change on hover.

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

#ifndef ANIMATEDBUTTON_H
#define ANIMATEDBUTTON_H

#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <string>
#include <vector>

class AnimatedButton {

    private:

        Gtk::Button button;
        Glib::RefPtr<Gdk::Pixbuf> pixbuf_black;
        Glib::RefPtr<Gdk::Pixbuf> pixbuf_white;
        Gtk::Image image;

    public:

        AnimatedButton (const std::string& black_icon,
                        const std::string& white_icon,
                        int size,
                        sigc::slot<void> callback,
                        const std::vector<std::string>& css_classes);
        ~AnimatedButton ();

        // Return the base button.
        Gtk::Button& get_button ();

    private:

        // Focus in.
        bool on_focus_in ();

        // Focus out
        bool on_focus_out ();

};

#endif

