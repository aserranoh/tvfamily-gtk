/*
mediasview.c - The view to choose a media.

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

#include "core.h"
#include "mainwindow.h"
#include "mediasview.h"
#include "paths.h"
#include "profilesview.h"

// CONSTANT DEFINITIONS

#define MEDIAS_BOX_NUM_COLS 4

// TYPE DEFINITION

MediasView medias_view;

// PRIVATE FUNCTIONS

static gboolean
medias_view_update_profile_picture (PictureRequest *r)
{
    GdkPixbuf *p = NULL;
    int size = gtk_widget_get_allocated_height (main_window.window) / 10;

    if (!r->error) {
        if (r->picture->len > 0) {
            GInputStream *gis;
            gis = g_memory_input_stream_new_from_data (
                r->picture->data, r->picture->len, NULL);
            p = gdk_pixbuf_new_from_stream_at_scale (
                gis, size, size, TRUE, NULL, NULL);
            g_object_unref (gis);
        } else {
            p = gdk_pixbuf_new_from_file_at_scale (
                paths_get_default_picture(), size, size, TRUE, NULL);
        }
        if (p) {
            profilemenu_set_picture (&medias_view.profile_menu, p);
        } else {
            warnx ("error: cannot load image for profile %s", core.profile);
        }
    }
    request_picture_destroy (r);
    return FALSE;
}

static void
medias_view_set_profile_picture (PictureRequest *r)
{
    g_idle_add ((GSourceFunc)medias_view_update_profile_picture, r);
}

static gboolean
medias_view_update_poster_box (PictureRequest *r)
{
    request_picture_destroy (r);
    return FALSE;
}

static void
medias_view_set_poster_box (PictureRequest *r)
{
    g_idle_add ((GSourceFunc)medias_view_update_poster_box, r);
}

static void
medias_view_update_medias (MediasRequest *r)
{
    // Check that the current category is the one we asked for
    if (r->category == gtk_button_get_label (
        GTK_BUTTON (medias_view.current_category)))
    {
        if (r->error) {
            gtk_widget_show (medias_view.label);
            gtk_stack_set_visible_child (
                GTK_STACK (medias_view.stack), medias_view.label);
        } else {
            if (request_medias_size (r)) {
                mediasbox_set (&medias_view.medias_box, r->medias);
                for (int i = 0; i < request_medias_size (r); i++) {
                    core_request_poster (
                        request_medias_get (r, i), medias_view_set_poster_box);
                }
                gtk_widget_show_all (medias_view.medias_box.box);
                gtk_stack_set_visible_child_name (
                    GTK_STACK (medias_view.stack), "medias");
                mediasbox_select (&medias_view.medias_box, 0);
            } else {
                // Show a mesage that no medias are available
                gtk_widget_show (medias_view.label);
                gtk_stack_set_visible_child (
                    GTK_STACK (medias_view.stack), medias_view.label);
            }
        }
    }
    request_medias_destroy (r);
}

static void
medias_view_set_medias (MediasRequest *r)
{
    g_idle_add ((GSourceFunc)medias_view_update_medias, r);
}

static void
medias_view_category_clicked (GtkWidget *widget, gpointer user_data)
{
    GtkStyleContext *context;

    // Update the current category button
    if (medias_view.current_category) {
        context = gtk_widget_get_style_context (medias_view.current_category);
        gtk_style_context_remove_class(context, "current-category");
    }
    medias_view.current_category = widget;
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class(context, "current-category");
    // Retrieve the list of medias
    if (gtk_widget_is_visible (medias_view.box)) {
        core_request_medias (
            gtk_button_get_label (GTK_BUTTON(widget)), medias_view_set_medias);
    }
}

static void
medias_view_update_categories (CategoriesRequest *r)
{
    if (!r->error) {
        size_t size = request_categories_size (r);
        medias_view.category_buttons = g_ptr_array_sized_new (size);
        for (int i = 0; i < size; i++) {
            GtkWidget *b = xbutton (request_categories_get (r, i),
                medias_view_category_clicked, NULL, "bar-button");
            g_ptr_array_add (medias_view.category_buttons, b);
            gtk_widget_show (b);
            menubar_add_front (&medias_view.bar, b);
        }
        if (size) {
            medias_view_category_clicked (
                g_ptr_array_index (medias_view.category_buttons, 0), NULL);
        }
    }
    request_categories_destroy (r);
}

static void
medias_view_set_categories (CategoriesRequest *r)
{
    g_idle_add ((GSourceFunc)medias_view_update_categories, r);
}

static void
medias_view_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    core_request_profile_picture (
        core.profile, medias_view_set_profile_picture);
    profilemenu_set_name (&medias_view.profile_menu, core.profile);
    if (medias_view.current_category) {
        medias_view_category_clicked (medias_view.current_category, NULL);
    }
    if (medias_view.category_buttons->len) {
        gtk_widget_grab_focus (
            g_ptr_array_index (medias_view.category_buttons, 0));
    }
}

static void
medias_view_leave (const char *next_view)
{
    //gtk_widget_hide (medias_view.stack);
    profilemenu_set_picture (&medias_view.profile_menu, NULL);
    main_window_change_view (next_view, NULL);
}

static void
medias_view_change_profile_picture (GtkWidget *widget, gpointer user_data)
{
    medias_view_leave ("change-picture");
}

static void
medias_view_settings (GtkWidget *widget, gpointer user_data)
{
    printf ("settings\n");
}

static void
medias_view_change_profile (GtkWidget *widget, gpointer user_data)
{
    core_set_profile (NULL);
    medias_view_leave ("choose-profile");
}

static void
medias_view_delete_profile (GtkWidget *widget, gpointer user_data)
{
    char *msg = g_strdup_printf ("Delete the profile %s?", core.profile);
    int response = message_new (main_window.window, MESSAGE_QUESTION, msg);
    g_free (msg);
    if (response == GTK_RESPONSE_YES) {
        if (core_delete_profile ()) {
            message_new (main_window.window, MESSAGE_ERROR,
                "Cannot delete the profile");
        } else {
            //profiles_view_clear_profiles ();
            medias_view_change_profile (NULL, NULL);
        }
    }
}

static void
medias_view_quit (GtkWidget *widget, gpointer user_data)
{
    exit_clicked (NULL, main_window.window);
}

static void
medias_view_build_poster_area (GtkWidget *box)
{
    GtkWidget *poster_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 15);
    GtkStyleContext *context = gtk_widget_get_style_context (poster_box);
    gtk_style_context_add_class (context, "poster-box");
    gtk_box_pack_start (GTK_BOX (box), poster_box, FALSE, FALSE, 0);

    // Title label
    medias_view.title_label = xlabel ("", "title-label", NULL);
    gtk_box_pack_start (
        GTK_BOX (poster_box), medias_view.title_label, FALSE, FALSE, 0);

    // Attrs
    GtkWidget *attrs_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start (GTK_BOX (poster_box), attrs_box, FALSE, FALSE, 0);
    medias_view.year_label = xlabel ("", "view-label", NULL);
    gtk_box_pack_start (
        GTK_BOX (attrs_box), medias_view.year_label, FALSE, FALSE, 0);
    gtk_box_pack_start (
        GTK_BOX (attrs_box), xlabel ("|", "sep", NULL), FALSE, FALSE, 0);
    medias_view.genre_label = xlabel ("", "view-label", NULL);
    gtk_box_pack_start (
        GTK_BOX (attrs_box), medias_view.genre_label, FALSE, FALSE, 0);
    gtk_box_pack_start (
        GTK_BOX (attrs_box), xlabel ("|", "sep", NULL), FALSE, FALSE, 0);
    GtkWidget *rating_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    const char *starfile = paths_get_image ("star");
    GtkWidget *img = gtk_image_new_from_file (starfile);
    g_free ((char *)starfile);
    medias_view.rating_label = xlabel ("", "view-label", "rating", NULL);
    gtk_box_pack_start (GTK_BOX (rating_box), img, FALSE, FALSE, 0);
    gtk_box_pack_start (
        GTK_BOX (rating_box), medias_view.rating_label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (attrs_box), rating_box, FALSE, FALSE, 0);

    // Hbox that contains the poster and the plot
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_box_pack_start (GTK_BOX (poster_box), hbox, FALSE, FALSE, 0);

    // Poster image
    medias_view.poster_image = gtk_image_new ();
    gtk_box_pack_start (
        GTK_BOX (hbox), medias_view.poster_image, FALSE, FALSE, 0);

    // Plot
    // Put it in an Vbox to align it to the top
    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_end (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
    medias_view.plot_label = xlabel ("", "view-label", NULL);
    gtk_label_set_line_wrap (GTK_LABEL (medias_view.plot_label), TRUE);
    gtk_label_set_justify (
        GTK_LABEL (medias_view.plot_label), GTK_JUSTIFY_FILL);
    gtk_box_pack_start (
        GTK_BOX (vbox), medias_view.plot_label, FALSE, FALSE, 0);
}

static void
medias_view_media_clicked (GtkWidget *widget, gpointer user_data)
{

}

static void
medias_view_media_focused (GtkWidget *widget, gpointer user_data)
{

}

static void
medias_view_build_medias_area (GtkWidget *box)
{
    mediasbox_create (&medias_view.medias_box, MEDIAS_BOX_NUM_COLS,
        medias_view_media_clicked, medias_view_media_focused);
    gtk_box_pack_end (
        GTK_BOX (box), medias_view.medias_box.box, FALSE, FALSE, 0);
}

// PUBLIC FUNCTIONS

int
medias_view_create ()
{
    int bar_height = gtk_widget_get_allocated_height (main_window.window) / 10;

    // Initialize attributes
    medias_view.current_category = NULL;

    // Create the main box
    medias_view.box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    // Prepare the menu bar
    GtkCallback callbacks[] = {
        medias_view_change_profile_picture,
        medias_view_settings,
        medias_view_change_profile,
        medias_view_delete_profile,
        medias_view_quit
    };
    if (menubar_create (&medias_view.bar, bar_height)) {
        return -1;
    }
    gtk_box_pack_start (
        GTK_BOX (medias_view.box), medias_view.bar.box, FALSE, FALSE, 0);
    profilemenu_create (&medias_view.profile_menu, bar_height, callbacks);
    menubar_add_back (&medias_view.bar, medias_view.profile_menu.button);

    // Add a stack to alternate between a message and the list of medias
    medias_view.stack = gtk_stack_new ();
    gtk_box_pack_start (
        GTK_BOX (medias_view.box), medias_view.stack, TRUE, TRUE, 0);

    // The label to display a message
    medias_view.label = xlabel ("No available medias", "view-label", NULL);
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    medias_view_build_poster_area (hbox);
    medias_view_build_medias_area (hbox);

    // Add the different contents to the stack
    gtk_stack_add_named (
        GTK_STACK (medias_view.stack), medias_view.label, "label");
    gtk_stack_add_named (GTK_STACK (medias_view.stack), hbox, "medias");

    // Show all elements and then hide the main box
    gtk_widget_show_all (medias_view.box);
    gtk_widget_hide (medias_view.box);

    // Request the list of categories
    core_request_categories(medias_view_set_categories);

    // Connect signals
    g_signal_connect (
        medias_view.box, "show", G_CALLBACK (medias_view_show), NULL);
    return 0;
}

void
medias_view_destroy ()
{
    mediasbox_destroy (&medias_view.medias_box);
    g_ptr_array_free (medias_view.category_buttons, TRUE);
    gtk_widget_destroy (medias_view.box);
}

