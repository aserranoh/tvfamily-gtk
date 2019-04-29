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

#define QUERY_CATEGORIES_TIMEOUT    1
#define MEDIAS_BOX_NUM_COLS         5
#define POSTER_BORDER               4
#define POSTER_RATIO                268/182

// TYPE DEFINITION

MediasView medias_view;

// FORWARD DECLARATION

static void
medias_view_set_categories (CategoriesRequest *r);

// PRIVATE FUNCTIONS

static gboolean
medias_view_update_profile_picture (PictureRequest *r)
{
    GdkPixbuf *p = NULL;
    int size = main_window_get_height () / 10;

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
medias_view_update_poster (PictureRequest *r)
{
    GdkPixbuf *p = NULL;
    GInputStream *gis;

    if (r->error) {
        p = gdk_pixbuf_new_from_file_at_scale (paths_get_default_picture(),
            medias_view.medias_box.poster_w, medias_view.medias_box.poster_h,
            FALSE, NULL);
    } else {
        if (r->picture->len > 0) {
            gis = g_memory_input_stream_new_from_data (
                r->picture->data, r->picture->len, NULL);
            p = gdk_pixbuf_new_from_stream_at_scale (gis,
                medias_view.medias_box.poster_w,
                medias_view.medias_box.poster_h, FALSE, NULL, NULL);
            if (!p) {
                warnx ("medias_view_update_poster: cannot load poster for "
                    "title '%s'", ((Media *)r->data)->title_id);
                p = gdk_pixbuf_new_from_file_at_scale (
                    paths_get_default_picture(),
                    medias_view.medias_box.poster_w,
                    medias_view.medias_box.poster_h, FALSE, NULL);
            }
            g_object_unref (gis);
        } else {
            p = gdk_pixbuf_new_from_file_at_scale (paths_get_default_picture(),
                medias_view.medias_box.poster_w,
                medias_view.medias_box.poster_h, FALSE, NULL);
        }
    }
    mediasbox_set_poster (&medias_view.medias_box, r->data, p);
    request_picture_destroy (r);
    return FALSE;
}

static void
medias_view_set_poster_box (PictureRequest *r)
{
    g_idle_add ((GSourceFunc)medias_view_update_poster, r);
}

static void
medias_view_set_medias_list (GPtrArray *medias)
{
    Media *m;

    if (medias->len) {
        if (mediasbox_set_medias (
            &medias_view.medias_box, medias))
        {
            GHashTable *h = g_hash_table_new (g_str_hash, g_str_equal);
            for (int i = 0; i < medias->len; i++) {
                // Keep a set with the requested poster and don't do
                // repeated requests
                m = g_ptr_array_index (medias, i);
                if (g_hash_table_add (h, m->title_id)) {
                    core_request_poster (
                        m, medias_view_set_poster_box);
                }
            }
            g_hash_table_unref (h);
        }
        gtk_widget_show_all (medias_view.medias_box.box);
        gtk_stack_set_visible_child_name (
            GTK_STACK (medias_view.stack), "medias");
        mediasbox_select (&medias_view.medias_box, 0);
    } else {
        // Show a mesage that no medias are available
        gtk_label_set_text (
            GTK_LABEL (medias_view.label), "No medias available");
        gtk_widget_show (medias_view.label);
        gtk_stack_set_visible_child (
            GTK_STACK (medias_view.stack), medias_view.label);
    }
}

static gboolean
medias_view_update_medias (MediasRequest *r)
{
    // Check that the current category is the one we asked for
    if (strcmp (r->category, gtk_button_get_label (
        GTK_BUTTON (medias_view.current_category))) == 0)
    {
        if (r->error) {
            gtk_label_set_text (
                GTK_LABEL (medias_view.label), "No medias available");
            gtk_widget_show (medias_view.label);
            gtk_stack_set_visible_child (
                GTK_STACK (medias_view.stack), medias_view.label);
        } else {
            medias_view_set_medias_list (r->medias);
        }
    }
    request_medias_destroy (r);
    return FALSE;
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
                medias_view_category_clicked, NULL, "bar-element",
                "bar-button", "bar-button-raw", NULL);
            g_ptr_array_add (medias_view.category_buttons, b);
            gtk_widget_show (b);
            menubar_add_front (&medias_view.bar, b);
        }
        if (size) {
            medias_view_category_clicked (
                g_ptr_array_index (medias_view.category_buttons, 0), NULL);
        }
    } else {
        g_timeout_add_seconds (QUERY_CATEGORIES_TIMEOUT,
            (GSourceFunc)core_request_categories, medias_view_set_categories);
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
    if (medias_view.category_buttons && medias_view.category_buttons->len) {
        gtk_widget_grab_focus (
            g_ptr_array_index (medias_view.category_buttons, 0));
    }
    // Compute the medias box's poster size
    int w = main_window_get_width () / MEDIAS_BOX_NUM_COLS
        - (2 * POSTER_BORDER);
    int h = w * POSTER_RATIO;
    mediasbox_set_poster_size (&medias_view.medias_box, w, h);
}

static void
medias_view_leave (const char *next_view)
{
    gtk_widget_hide (medias_view.medias_box.box);
    gtk_widget_hide (medias_view.label);
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
medias_view_media_clicked (GtkWidget *widget, gpointer user_data)
{
    printf("media clicked\n");
}

static gboolean
medias_view_update_search_result (SearchRequest *r)
{
    if (r->error) {
        gtk_label_set_text (
            GTK_LABEL (medias_view.label), "No titles available");
    } else {
        medias_view_set_medias_list (r->result);
    }
    request_search_destroy (r);
    return FALSE;
}

static void
medias_view_set_search_result (SearchRequest *r)
{
    g_idle_add ((GSourceFunc)medias_view_update_search_result, r);
}

static void
medias_view_search_activated (GtkWidget *widget, gpointer user_data)
{
    gtk_label_set_text (GTK_LABEL (medias_view.label), "Searching...");
    gtk_widget_show (medias_view.label);
    gtk_stack_set_visible_child_name (GTK_STACK (medias_view.stack), "label");
    core_request_search (
        gtk_button_get_label (GTK_BUTTON (medias_view.current_category)),
        gtk_entry_get_text (GTK_ENTRY (medias_view.search_entry)),
        medias_view_set_search_result);
}

static void
medias_view_destroyed (GtkWidget *widget, gpointer user_data)
{
    if (medias_view.category_buttons) {
        g_ptr_array_free (medias_view.category_buttons, TRUE);
    }
}

// PUBLIC FUNCTIONS

int
medias_view_create ()
{
    int bar_height = main_window_get_height () / 10;

    // Initialize attributes
    medias_view.current_category = NULL;
    medias_view.category_buttons = NULL;

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
    medias_view.search_entry = gtk_search_entry_new ();
    gtk_widget_set_name (medias_view.search_entry, "search-entry");
    gtk_widget_set_size_request (medias_view.search_entry, 300, -1);
    g_signal_connect (medias_view.search_entry, "activate",
        G_CALLBACK (medias_view_search_activated), NULL);
    menubar_add_back (&medias_view.bar, medias_view.search_entry);

    // Add a stack to alternate between a message and the list of medias
    medias_view.stack = gtk_stack_new ();
    gtk_box_pack_start (
        GTK_BOX (medias_view.box), medias_view.stack, TRUE, TRUE, 0);

    // The label to display a message
    medias_view.label = xlabel ("No available medias", "view-label", NULL);
    mediasbox_create (&medias_view.medias_box, MEDIAS_BOX_NUM_COLS,
        G_CALLBACK (medias_view_media_clicked));

    // Add the different contents to the stack
    gtk_stack_add_named (
        GTK_STACK (medias_view.stack), medias_view.label, "label");
    gtk_stack_add_named (
        GTK_STACK (medias_view.stack), medias_view.medias_box.box, "medias");

    // Show all elements and then hide the main box
    gtk_widget_show_all (medias_view.box);
    gtk_widget_hide (medias_view.box);
    gtk_widget_hide (medias_view.label);

    // Request the list of categories
    core_request_categories(medias_view_set_categories);

    // Connect signals
    g_signal_connect (
        medias_view.box, "show", G_CALLBACK (medias_view_show), NULL);
    g_signal_connect (
        medias_view.box, "destroy", G_CALLBACK (medias_view_destroyed), NULL);
    return 0;
}

