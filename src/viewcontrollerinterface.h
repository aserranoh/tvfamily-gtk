/*
viewcontrollerinterface.h - Interface with methods of ViewSwitcher.

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

#ifndef VIEWCONTROLLERINTERFACE_H
#define VIEWCONTROLLERINTERFACE_H

#include <gtkmm/window.h>
#include <string>

#include "core.h"
#include "viewswitchdata.h"

class ViewControllerInterface {

    public:

        /* Return the application's core. */
        virtual Core& get_core () = 0;

        /* Return the main window. */
        virtual Gtk::Window& get_window () = 0;

        /* Switch the current view to new_view, without passing any data. */
        virtual void switch_view (const std::string& new_view) = 0;

        /* Switch the current view to new_view. It passes some data in the
           process. */
        virtual void switch_view (const std::string& new_view,
                                  const ViewSwitchData& data) = 0;

        /* Switch the current view to the previous view. */
        virtual void back () = 0;

        /* Return the name of the current view. */
        virtual Gtk::Widget* get_current_view () const;

        /* Exit from the main loop in the GTK application. */
        virtual void exit () = 0;

};

#endif

