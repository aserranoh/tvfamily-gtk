/*
mediasview.h - The view to choose a media.

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

#ifndef MEDIASVIEW_H
#define MEDIASVIEW_H

#include <gtkmm/window.h>

#include "view.h"

/*typedef struct MediasView_s {
    GtkWidget *box;
    MenuBar bar;
    ProfileMenu profile_menu;
    GtkWidget *stack;
    GtkWidget *label;
    GPtrArray *category_buttons;
    GtkWidget *current_category;
    MediasBox medias_box;
    GtkWidget *search_entry;
} MediasView_t;

extern MediasView_t medias_view;

int
medias_view_create ();*/

class MediasView: public View {

    public:

        MediasView (ViewControllerInterface& controller);
        ~MediasView ();

        // Pass some data to this view.
        void set_data (const ViewSwitchData& data);

};

#endif

