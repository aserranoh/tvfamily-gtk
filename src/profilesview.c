/*
profilesview.c - The view to choose a profile.

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

#include <err.h>

#include "profilesview.h"
#include "core.h"
#include "mainwindow.h"
#include "paths.h"

#define PROFILE_PICTURE_SIZE    128
#define QUERY_PROFILES_TIMEOUT  5

#define BAR_ICON_MARGIN         5

// TYPE DEFINITION

ProfilesView profiles_view;

// FORWARD DECLARATIONS

static void
profiles_view_set_profiles (ProfilesRequest *r);

// PRIVATE FUNCTIONS

static void
profiles_view_exit (GtkWidget *widget, gpointer user_data)
{
    exit_clicked (widget, main_window.window);
}

static GtkWidget *
profiles_view_new_exit_button ()
{
    int size = profiles_view.bar.height;
    // Create the elements for the button
    const char *paths[] = {
        paths_get_image ("off-black"),
        paths_get_image ("off-white")
    };
    GtkWidget *button = anibutton (paths, size - 2*BAR_ICON_MARGIN,
        profiles_view_exit, NULL, "bar-element", "bar-button",
        "bar-icon-button", NULL);
    g_free ((char *)paths[0]);
    g_free ((char *)paths[1]);
    return button;
}

static gboolean
profiles_view_update_picture (PictureRequest *r)
{
    GdkPixbuf *p = NULL;
    GInputStream *gis;

    if (!r->error) {
        if (r->picture->len > 0) {
            gis = g_memory_input_stream_new_from_data (
                r->picture->data, r->picture->len, NULL);
            p = gdk_pixbuf_new_from_stream_at_scale (gis,
                PROFILE_PICTURE_SIZE, PROFILE_PICTURE_SIZE, TRUE, NULL, NULL);
            g_object_unref (gis);
        } else {
            p = gdk_pixbuf_new_from_file_at_scale (paths_get_default_picture(),
                PROFILE_PICTURE_SIZE, PROFILE_PICTURE_SIZE, TRUE, NULL);
        }
        const char *profile = (const char *)r->data;
        if (p) {
            profilesbox_set_picture (
                &profiles_view.profiles_box, profile, p);
        } else {
            warnx ("error: cannot load image for profile %s", profile);
        }
    }
    g_free (r->data);
    request_picture_destroy (r);
    return FALSE;
}

static void
profiles_view_set_picture (PictureRequest *r)
{
    g_idle_add ((GSourceFunc)profiles_view_update_picture, r);
}

static void
profiles_view_set_default_focus ()
{
    /* If the focus hasn't been given to a profile yet, make a profile have
       the focus, if any. If not, give the focus to the create_profile_button.
    */
    if (!profiles_view.profile_get_focus) {
        if (!profilesbox_set_focus (&profiles_view.profiles_box, 0)) {
            profiles_view.profile_get_focus = 1;
        } else {
            if (!profiles_view.button_get_focus) {
                gtk_widget_grab_focus (profiles_view.new_profile_button);
                profiles_view.button_get_focus = 1;
            }
        }
    }
}

static void
profiles_view_show_label (const char *label)
{
    gtk_label_set_text (GTK_LABEL (profiles_view.label), label);
    gtk_widget_show (profiles_view.label);
    gtk_stack_set_visible_child (GTK_STACK (profiles_view.stack),
        profiles_view.label);
}

static gboolean
profiles_view_update_profiles (ProfilesRequest *r)
{
    if (r->error) {
        // Some error ocurred, show the message
        profiles_view_show_label ("Cannot get list of profiles");
    } else {
        if (!request_profiles_size (r)) {
            // No profiles available
            profiles_view_show_label ("No profiles available");
        } else {
            // Set the profiles list
            int changes = profilesbox_set (
                &profiles_view.profiles_box, r->profiles);
            /* Request the profiles pictures only if there's has been some
               change in the profiles list. */
            for (int i = 0; i < request_profiles_size (r); i++) {
                core_request_profile_picture (
                    request_profiles_get (r, i), profiles_view_set_picture);
            }
            gtk_widget_show_all (profiles_view.profiles_box.box);
            gtk_stack_set_visible_child (GTK_STACK (profiles_view.stack),
                profiles_view.profiles_box.box);
        }
    }
    profiles_view_set_default_focus ();
    request_profiles_destroy (r);

    // Request again the list of profiles after a given timeout
    profiles_view.qid = g_timeout_add_seconds (QUERY_PROFILES_TIMEOUT,
        (GSourceFunc)core_request_profiles, profiles_view_set_profiles);
    return FALSE;
}

static void
profiles_view_set_profiles (ProfilesRequest *r)
{
    g_idle_add ((GSourceFunc)profiles_view_update_profiles, r);
}

static void
profiles_view_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    core_request_profiles (profiles_view_set_profiles);
}

static void
profiles_view_leave (const char *next_view)
{
    g_source_remove (profiles_view.qid);
    profiles_view.profile_get_focus = 0;
    profiles_view.button_get_focus = 0;
    gtk_widget_hide (profiles_view.profiles_box.box);
    main_window_change_view (next_view, NULL);
}

static void
profiles_view_new_profile (GtkWidget *widget, gpointer user_data)
{
    profiles_view_leave ("new-profile");
}

static void
profiles_view_profile_clicked (GtkWidget *widget, gpointer user_data)
{
    core_set_profile ((const char *)user_data);
    profiles_view_leave ("medias");
}

int
profiles_view_create ()
{
    int bar_height = main_window_get_height () / 10;
    GtkWidget *exit_button;
    GtkWidget *contents_box;
    GtkWidget *hbox;

    // Initialize attributes
    profiles_view.profile_get_focus = 0;
    profiles_view.button_get_focus = 0;
    profiles_view.qid = 0;

    // Create the main box
    profiles_view.box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // Prepare the menu bar
    if (menubar_create (&profiles_view.bar, bar_height)) {
        return -1;
    }
    gtk_box_pack_start (GTK_BOX (profiles_view.box),
        profiles_view.bar.box, FALSE, FALSE, 0);
    exit_button = profiles_view_new_exit_button ();
    menubar_add_back (&profiles_view.bar, exit_button);

    // Create the contents
    contents_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 20);
    gtk_box_pack_start (
        GTK_BOX (profiles_view.box), contents_box, TRUE, TRUE, 0);
    gtk_widget_set_valign (contents_box, GTK_ALIGN_CENTER);
    // The profiles box
    profilesbox_create (&profiles_view.profiles_box, PROFILE_PICTURE_SIZE,
        profiles_view_profile_clicked);
    // The label for the messages if no profiles
    profiles_view.label = xlabel ("", "view-label", 0);
    // The stack to alternate between the label and the list of profiles
    profiles_view.stack = gtk_stack_new ();
    gtk_box_pack_start (
        GTK_BOX (contents_box), profiles_view.stack, TRUE, TRUE, 0);
    gtk_stack_add_named (GTK_STACK (profiles_view.stack),
        profiles_view.profiles_box.box, "profiles");
    gtk_stack_add_named (GTK_STACK (profiles_view.stack),
        profiles_view.label, "label");
    /* Create an horizontal box that contain the 'New profile button', because
       if it is put in the vertical box it expandes horizontally, and we don't
       want that */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (contents_box), hbox, FALSE, FALSE, 0);
    // Create the 'New profile button'
    profiles_view.new_profile_button = viewbutton (
        "New profile", profiles_view_new_profile, NULL);
    gtk_box_pack_start (GTK_BOX (hbox), profiles_view.new_profile_button,
        TRUE, FALSE, 0);

    // Show all elements and then hide the main box
    gtk_widget_show_all (profiles_view.box);
    gtk_widget_hide (profiles_view.box);

    // Connect signals
    g_signal_connect (profiles_view.box, "show",
        G_CALLBACK (profiles_view_show), NULL);
    return 0;
}

void
profiles_view_clear_profiles ()
{
    profilesbox_clear (&profiles_view.profiles_box);
}

void
profiles_view_destroy ()
{
    gtk_widget_destroy (profiles_view.box);
}

