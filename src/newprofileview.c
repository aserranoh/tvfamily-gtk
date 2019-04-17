/*
newprofileview.c - The view to create a profile.

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
#include "newprofileview.h"

#define CROP_IMAGE_W    320
#define CROP_IMAGE_H    240

// TYPE DEFINITION

NewProfileView newprofile_view;

// PRIVATE FUNCTIONS

static void
newprofile_view_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    gtk_button_set_label (GTK_BUTTON (newprofile_view.filebutton), "None");
    gtk_widget_grab_focus (newprofile_view.entry);
}

// PUBLIC FUNCTIONS

static void
newprofile_view_leave ()
{
    cropimage_set_image (&newprofile_view.crop_image, NULL);
    gtk_entry_set_text (GTK_ENTRY (newprofile_view.entry), "");
    main_window_back ();
}

static void
newprofile_view_back (GtkWidget *widget, gpointer user_data)
{
    newprofile_view_leave ();
}

static void
newprofile_view_choose_picture (GtkWidget *widget, gpointer user_data)
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
        if (cropimage_set_image (&newprofile_view.crop_image, filename)) {
            message_new (main_window.window, MESSAGE_ERROR,
                "Picture format not supported");
        } else {
            gtk_button_set_label (
                GTK_BUTTON (newprofile_view.filebutton), basename (filename));
        }
    }
    g_free (filename);
}

static void
newprofile_view_action (GtkWidget *widget, gpointer user_data)
{
    GdkPixbuf *p;
    char *errstr;

    if (!gtk_entry_get_text_length (GTK_ENTRY (newprofile_view.entry))) {
        message_new (main_window.window, MESSAGE_ERROR,
            "The profile name cannot be empty");
        return;
    }
    p = cropimage_get_cropped_image (&newprofile_view.crop_image);
    if (core_create_profile (
        gtk_entry_get_text (GTK_ENTRY (newprofile_view.entry)), p, &errstr))
    {
        char *s = g_strdup_printf (
            "Cannot create the profile: %s", errstr);
        message_new (main_window.window, MESSAGE_ERROR, s);
        g_free (s);
        g_free (errstr);
    } else {
        newprofile_view_leave ();
    }
    if (p) {
        g_object_unref (p);
    }
}

int
newprofile_view_create ()
{
    int bar_height = main_window_get_height () / 10;

    // Create the main box
    newprofile_view.box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // Prepare the menu bar
    if (menubar_create (&newprofile_view.bar, bar_height)) {
        return -1;
    }
    gtk_box_pack_start (GTK_BOX (newprofile_view.box),
        newprofile_view.bar.box, FALSE, FALSE, 0);
    GtkWidget *back = xbutton ("Back", newprofile_view_back, NULL,
        "bar-element", "bar-button", "bar-button-raw", NULL);
    gtk_widget_set_size_request (back, 120, -1);
    menubar_add_back (&newprofile_view.bar, back);

    // Configure the contents box
    GtkWidget *contents_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name (contents_box, "new-profile-contents-box");
    gtk_box_pack_start (
        GTK_BOX (newprofile_view.box), contents_box, TRUE, TRUE, 0);
    gtk_widget_set_valign (contents_box, GTK_ALIGN_CENTER);
    GtkWidget *hbox_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign (contents_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (contents_box), hbox_main, FALSE, FALSE, 0);
    GtkWidget *vbox_main = gtk_box_new (GTK_ORIENTATION_VERTICAL, 30);
    gtk_box_pack_start (GTK_BOX (hbox_main), vbox_main, FALSE, FALSE, 0);

    // Create the controls for the profile's name
    GtkWidget *vbox_name = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start (GTK_BOX (vbox_main), vbox_name, FALSE, FALSE, 0);
    GtkWidget *hbox_label_name = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_name), hbox_label_name, FALSE, FALSE, 0);
    GtkWidget *label = xlabel ("New profile's name", "view-label", 0);
    gtk_box_pack_start (GTK_BOX (hbox_label_name), label, FALSE, FALSE, 0);
    newprofile_view.entry = gtk_entry_new ();
    gtk_box_pack_start (
        GTK_BOX (vbox_name), newprofile_view.entry, FALSE, FALSE, 0);

    // Create a button to choose the profile picture
    GtkWidget *vbox_file = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start (GTK_BOX (vbox_main), vbox_file, FALSE, FALSE, 0);
    GtkWidget *hbox_label_file = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox_file), hbox_label_file, FALSE, FALSE, 0);
    newprofile_view.picture_label = xlabel (
        "Profile picture", "view-label", 0);
    gtk_box_pack_start (GTK_BOX (hbox_label_file),
        newprofile_view.picture_label, FALSE, FALSE, 0);
    newprofile_view.filebutton = viewbutton (
        "None", newprofile_view_choose_picture, NULL);
    gtk_widget_set_size_request (newprofile_view.filebutton, 600, -1);
    gtk_box_pack_start (
        GTK_BOX (vbox_file), newprofile_view.filebutton, FALSE, FALSE, 0);

    // Create the control to crop the selected picture
    cropimage_create (&newprofile_view.crop_image, CROP_IMAGE_W, CROP_IMAGE_H);
    gtk_box_pack_start (
        GTK_BOX (vbox_main), newprofile_view.crop_image.box, FALSE, FALSE, 0);

    // Create the 'Create' button
    GtkWidget *hbox_create = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (
        GTK_BOX (vbox_main), hbox_create, FALSE, FALSE, 0);
    newprofile_view.action_button = viewbutton (
        "Create", newprofile_view_action, NULL);
    gtk_box_pack_start (
        GTK_BOX (hbox_create), newprofile_view.action_button, TRUE, FALSE, 0);
    
    // Show all elements and then hide the main box
    gtk_widget_show_all (newprofile_view.box);
    gtk_widget_hide (newprofile_view.box);

    // Connect signals
    g_signal_connect (
        newprofile_view.box, "show", G_CALLBACK (newprofile_view_show), NULL);
    return 0;
}

