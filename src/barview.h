/*
barview.h - A generic view with a menu bar.

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

#ifndef BARVIEW_H
#define BARVIEW_H

#include "menubar.h"
#include "view.h"
#include "viewcontrollerinterface.h"

class BarView: public View {

    private:

        static const float BAR_RATIO;

        // The menu bar
        MenuBar bar;

    public:

        BarView (ViewControllerInterface& controller);
        virtual ~BarView ();

        // Return the bar.
        MenuBar& get_bar ();

};

#endif

