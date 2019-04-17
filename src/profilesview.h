/*
profilesview.h - The view to choose a profile.

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

#ifndef PROFILESVIEW_H
#define PROFILESVIEW_H

#include <gtk/gtk.h>

#include "widgets.h"

typedef struct ProfilesView_s {
    GtkWidget *box;
    MenuBar bar;
    ProfilesBox profiles_box;
    GtkWidget *label;
    GtkWidget *stack;
    GtkWidget *new_profile_button;
    int profile_get_focus;
    int button_get_focus;
} ProfilesView;

extern ProfilesView profiles_view;

int
profiles_view_create ();

#endif

