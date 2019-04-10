/*
pictureview.c - The view to change a profile picture.

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

#include <libgen.h>

#include "mainwindow.h"
#include "pictureview.h"

#define CROP_IMAGE_W    320
#define CROP_IMAGE_H    240

// TYPE DEFINITION

PictureView picture_view;

// PRIVATE FUNCTIONS

static void
picture_view_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    gtk_button_set_label (GTK_BUTTON (picture_view.filebutton), "None");
    gtk_widget_grab_focus (picture_view.filebutton);
}

// PUBLIC FUNCTIONS

static void
picture_view_leave ()
{
    cropimage_set_image (&picture_view.crop_image, NULL);
    main_window_back ();
}

static void
picture_view_back (GtkWidget *widget, gpointer user_data)
{
    picture_view_leave ();
}

static void
picture_view_choose_picture (GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new (
        "Choose the profile picture", GTK_WINDOW (main_window.window),
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open",
        GTK_RESPONSE_OK, NULL);
    GtkFileFilter *filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "JPEG Images");
    gtk_file_filter_add_mime_type (filter, "image/jpeg");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, "PNG Images");
    gtk_file_filter_add_mime_type (filter, "image/png");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
    int response = gtk_dialog_run (GTK_DIALOG (dialog));
    char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    gtk_widget_destroy (dialog);
    if (response == GTK_RESPONSE_OK) {
        if (cropimage_set_image (&picture_view.crop_image, filename)) {
            message_new (main_window.window, MESSAGE_ERROR,
                "Picture format not supported");
        } else {
            gtk_button_set_label (
                GTK_BUTTON (picture_view.filebutton), basename (filename));
        }
    }
    g_free (filename);
}

static void
picture_view_action (GtkWidget *widget, gpointer user_data)
{
    GdkPixbuf *p;
    char *errstr;

    if (cropimage_get_cropped_image (&picture_view.crop_image, &p)) {
        message_new (main_window.window, MESSAGE_ERROR,
            "Cannot load the profile image");
        return;
    }
    if (core_set_profile_picture (p, &errstr))
    {
        char *s = g_strdup_printf (
            "Cannot change the profile picture: %s", errstr);
        message_new (main_window.window, MESSAGE_ERROR, s);
        g_free (s);
        g_free (errstr);
    } else {
        picture_view_leave ();
    }
    if (p) {
        g_object_unref (p);
    }
}

int
picture_view_create ()
{
    int bar_height = gtk_widget_get_allocated_height (main_window.window) / 10;

    // Create the main box
    picture_view.box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // Prepare the menu bar
    if (menubar_create (&picture_view.bar, bar_height)) {
        return -1;
    }
    gtk_box_pack_start (
        GTK_BOX (picture_view.box), picture_view.bar.box, FALSE, FALSE, 0);
    GtkWidget *back = xbutton ("Back", picture_view_back, NULL, "bar-button");
    gtk_widget_set_size_request (back, 120, -1);
    menubar_add_back (&picture_view.bar, back);

    // Configure the contents box
    GtkWidget *contents_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name (contents_box, "new-profile-contents-box");
    gtk_box_pack_start (
        GTK_BOX (picture_view.box), contents_box, TRUE, TRUE, 0);
    gtk_widget_set_valign (contents_box, GTK_ALIGN_CENTER);
    GtkWidget *hbox_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign (contents_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (contents_box), hbox_main, FALSE, FALSE, 0);
    GtkWidget *vbox_main = gtk_box_new (GTK_ORIENTATION_VERTICAL, 30);
    gtk_box_pack_start (GTK_BOX (hbox_main), vbox_main, FALSE, FALSE, 0);

    // Create a button to choose the profile picture
    GtkWidget *vbox_file = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start (GTK_BOX (vbox_main), vbox_file, FALSE, FALSE, 0);
    GtkWidget *hbox_label_file = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_file), hbox_label_file, FALSE, FALSE, 0);
    picture_view.picture_label = xlabel ("Profile picture", "view-label", 0);
    gtk_box_pack_start (GTK_BOX (hbox_label_file),
        picture_view.picture_label, FALSE, FALSE, 0);
    picture_view.filebutton = viewbutton (
        "None", picture_view_choose_picture, NULL);
    gtk_widget_set_size_request (picture_view.filebutton, 600, -1);
    gtk_box_pack_start (
        GTK_BOX (vbox_file), picture_view.filebutton, FALSE, FALSE, 0);

    // Create the control to crop the selected picture
    cropimage_create (&picture_view.crop_image, CROP_IMAGE_W, CROP_IMAGE_H);
    gtk_box_pack_start (
        GTK_BOX (vbox_main), picture_view.crop_image.box, FALSE, FALSE, 0);

    // Create the 'Save' button
    GtkWidget *hbox_save = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_main), hbox_save, FALSE, FALSE, 0);
    picture_view.action_button = viewbutton (
        "Save", picture_view_action, NULL);
    gtk_box_pack_start (
        GTK_BOX (hbox_save), picture_view.action_button, TRUE, FALSE, 0);
    
    // Show all elements and then hide the main box
    gtk_widget_show_all (picture_view.box);
    gtk_widget_hide (picture_view.box);

    // Connect signals
    g_signal_connect (
        picture_view.box, "show", G_CALLBACK (picture_view_show), NULL);
    return 0;
}

void
picture_view_destroy ()
{
    gtk_widget_destroy (picture_view.box);
}

