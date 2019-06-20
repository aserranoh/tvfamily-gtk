/*
newprofileview.h - The view to create a profile.

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

#ifndef NEWPROFILEVIEW_H
#define NEWPROFILEVIEW_H

#include <gtkmm/window.h>

#include "view.h"

class NewProfileView: public View {

    public:

        NewProfileView (ViewControllerInterface& controller);
        ~NewProfileView ();

        // Pass some data to this view.
        void set_data (const ViewSwitchData& data);

};

/*typedef struct NewProfileView_s {
    GtkWidget *box;
    MenuBar bar;
    GtkWidget *picture_label;
    GtkWidget *filebutton;
    CropImage crop_image;
    GtkWidget *action_button;
    GtkWidget *entry;
} NewProfileView;

extern NewProfileView newprofile_view;

int
newprofile_view_create ();*/

#endif

