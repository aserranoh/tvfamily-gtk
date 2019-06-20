/*
player.h - The video player.

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

#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <gtkmm/window.h>

#include "view.h"

/*typedef struct PlayerView_s {
    GtkWidget *box;
    Media *media;
    ProgressDialog progress;
} PlayerView;

extern PlayerView player_view;

int
player_view_create ();*/

class PlayerView: public View {

    public:

        PlayerView (ViewControllerInterface& controller);
        ~PlayerView ();

        // Pass some data to this view.
        void set_data (const ViewSwitchData& data);

};

#endif

