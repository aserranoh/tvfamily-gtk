/*
mediainfoview.cpp - The view with the detailed info of a media.

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

// PRIVATE FUNCTIONS

/*static void
mediainfo_view_leave (const char *next_view)
{
    main_window_change_view (next_view, mediainfo_view.media);
}

static void
mediainfo_view_play (GtkWidget *widget, gpointer user_data)
{
    mediainfo_view_leave ("player");
}

static void
mediainfo_view_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    mediainfo_view.media = (Media *)main_window_get_data ();
}

// PUBLIC FUNCTIONS

int
mediainfo_view_create ()
{
    int bar_height = main_window_get_height () / 10;

    // Create the main box
    mediainfo_view.box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // Prepare the menu bar
    if (menubar_create (&mediainfo_view.bar, bar_height)) {
        return -1;
    }
    gtk_box_pack_start (
        GTK_BOX (mediainfo_view.box), mediainfo_view.bar.box, FALSE, FALSE, 0);

    // Create the button to play the media
    GtkWidget *button = gtk_button_new_with_label ("Play");
    gtk_box_pack_start (GTK_BOX (mediainfo_view.box), button, FALSE, FALSE, 0);

    // Show all elements and then hide the main box
    gtk_widget_show_all (mediainfo_view.box);
    gtk_widget_hide (mediainfo_view.box);

    // Connect signals
    g_signal_connect (
        mediainfo_view.box, "show", G_CALLBACK (mediainfo_view_show), NULL);
    g_signal_connect (
        button, "clicked", G_CALLBACK (mediainfo_view_play), NULL);
    return 0;
}*/

#include "mediainfoview.h"

MediaInfoView::MediaInfoView (ViewControllerInterface& controller):
    View (controller)
{}

MediaInfoView::~MediaInfoView ()
{}

void MediaInfoView::set_data (const ViewSwitchData& data)
{}

