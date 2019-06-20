/*
view.h - Base class for all the views.

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

#ifndef VIEW_H
#define VIEW_H

#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <string>

#include "viewswitchdata.h"
#include "viewcontrollerinterface.h"

class View {

    private:

        // The box that contains all the elements in the view.
        Gtk::Box box;

        // A reference to the view switcher.
        ViewControllerInterface& controller;

    public:

        View (ViewControllerInterface& controller);
        virtual ~View ();

        // Return the Box that contains everything in this view.
        inline Gtk::Box& get_box ()
            { return box; }

        // Return the view controller.
        inline ViewControllerInterface& get_controller ()
            { return controller; }

        // Show this window
        virtual void show ();
        virtual void show (const ViewSwitchData& data);

};

#endif

