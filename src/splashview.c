/*
splashview.c - The view with the TVFamily logo.

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

#include "err.h"

#include "mainwindow.h"
#include "paths.h"
#include "splashview.h"

#define SPLASH_ICON_WIDTH   800
#define SPLASH_SECONDS      0

SplashView splash_view;

static void
splash_view_timeout_elapsed ()
{
    main_window_change_view ("choose-profile", NULL);
    gtk_widget_destroy (splash_view.image);
}

int
splash_view_create ()
{
    // Create the main box
    splash_view.box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // Load the pixbuf
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale (paths_get_logo (),
        SPLASH_ICON_WIDTH, -1, TRUE, NULL);
    if (!pixbuf) {
        warnx ("error: cannot load TVFamily logo");
        return -1;
    }

    // Create the image
    splash_view.image = gtk_image_new_from_pixbuf (pixbuf);
    gtk_box_pack_start (
        GTK_BOX (splash_view.box), splash_view.image, TRUE, FALSE, 0);

    // Change of view after some seconds
    g_timeout_add_seconds (
        SPLASH_SECONDS, (GSourceFunc)splash_view_timeout_elapsed, NULL);

    // Show all elements and then hide the main box
    gtk_widget_show_all (splash_view.box);
    gtk_widget_hide (splash_view.box);

    // Unref the pixbuf so that we give control to the image widget
    g_object_unref (pixbuf);
    return 0;
}

