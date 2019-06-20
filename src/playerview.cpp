/*
player.cpp - The video player.

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
player_view_play ()
{
    printf ("Playing starts!\n");
}

static gboolean
player_view_update_status ()
{
    MediaStatus status;
    gboolean repeat;

    // Get media status in the server
    mediastatus_init (&status);
    if (core_get_media_status (player_view.media, &status)) {
        // Error getting the file status
        gtk_dialog_response (
            GTK_DIALOG (player_view.progress.dialog), GTK_RESPONSE_NO);
        repeat = FALSE;
    } else {
        if (status.status == STATUS_ERROR) {
            gtk_dialog_response (
                GTK_DIALOG (player_view.progress.dialog), GTK_RESPONSE_NO);
            repeat = FALSE;
        } else if (status.status == STATUS_DOWNLOADED) {
            gtk_dialog_response (
                GTK_DIALOG (player_view.progress.dialog), GTK_RESPONSE_OK);
            repeat = FALSE;
        } else {
            progress_set_message (&player_view.progress, status.message);
            progress_set_progress (&player_view.progress, status.progress);
            repeat = TRUE;
        }
    }
    mediastatus_destroy (&status);
    return repeat;
}

static void
player_view_wait ()
{
    int response;

    // Create a progress modial dialog
    progress_create (&player_view.progress);
    g_timeout_add_seconds (1, (GSourceFunc)player_view_update_status, NULL);
    response = gtk_dialog_run (GTK_DIALOG (player_view.progress.dialog));
    progress_destroy (&player_view.progress);
    if (response == GTK_RESPONSE_OK) {
        player_view_play ();
    } else {
        message_new (main_window.window, MESSAGE_ERROR,
            "Error downloading the media.");
        main_window_back ();
    }
}

static void
player_view_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    MediaStatus status;
    char *errstr;

    // Get media status in the server
    mediastatus_init (&status);
    player_view.media = (Media *)main_window_get_data ();
    if (core_get_media_status (player_view.media, &status)) {
        // Error getting the file status
        message_new (main_window.window, MESSAGE_ERROR,
            "Cannot get media status.");
        main_window_back ();
    } else {
        if (status.status == STATUS_MISSING) {
            // Tell the server to download this media
            if (core_download_media (player_view.media, &errstr)) {
                // Error downloading the media
                char *s = g_strdup_printf (
                    "Error downloading the media: %s", errstr);
                message_new (main_window.window, MESSAGE_ERROR, s);
                g_free (s);
                g_free (errstr);
                main_window_back ();
            } else {
                player_view_wait ();
            }
        } else if (status.status == STATUS_DOWNLOADING) {
            // The media is still being downloaded
            player_view_wait ();
        } else {
            // Media already available
            player_view_play ();
        }
    }
    mediastatus_destroy (&status);
}

// PUBLIC FUNCTIONS

int
player_view_create ()
{
    // Create the main box
    player_view.box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // Show all elements and then hide the main box
    gtk_widget_show_all (player_view.box);
    gtk_widget_hide (player_view.box);

    // Connect signals
    g_signal_connect (
        player_view.box, "show", G_CALLBACK (player_view_show), NULL);
    return 0;
}*/

#include "playerview.h"

PlayerView::PlayerView (ViewControllerInterface& controller):
    View (controller)
{}

PlayerView::~PlayerView ()
{}

void PlayerView::set_data (const ViewSwitchData& data)
{}

