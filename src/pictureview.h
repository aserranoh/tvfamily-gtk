/*
pictureview.h - The view to change a profile picture.

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

#ifndef PICTUREVIEW_H
#define PICTUREVIEW_H

#include "core.h"
#include "widgets.h"

typedef struct PictureView_s {
    GtkWidget *box;
    MenuBar bar;
    GtkWidget *picture_label;
    GtkWidget *filebutton;
    CropImage crop_image;
    GtkWidget *action_button;
} PictureView;

extern PictureView picture_view;

int
picture_view_create ();

/* Destroy this view. */
void
picture_view_destroy ();

#endif

