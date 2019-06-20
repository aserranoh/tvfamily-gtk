/*
menubar.h - A menu bar on top of the views.

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

#ifndef MENUBAR_H
#define MENUBAR_H

#include <gtkmm/box.h>
#include <gtkmm/image.h>

class MenuBar {

    private:

        static const int LOGO_MARGIN = 5;

        Gtk::Box box;
        Gtk::Image logo;
        int height;

    public:

        MenuBar (int height);
        ~MenuBar ();

        // Return the container box of this menu bar.
        Gtk::Box& get_box ();

        // Return the height of this bar.
        int get_height () const;

        // Add a component to the back of this menu bar.
        void add_back (Gtk::Widget &widget);

};

#endif

