/*
main.c - This is the entry point to the application.

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
#include <getopt.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "core.h"
#include "mainwindow.h"
#include "mediasview.h"
#include "newprofileview.h"
#include "paths.h"
#include "pictureview.h"
#include "profilesview.h"
#include "splashview.h"

// Short options:
//   * h: help
//   * v: version
//   * a: server address
#define OPTSTRING   "hva:"

#define FULLSCREEN_KEY  GDK_KEY_F11

const char *server_address;
MainWindow main_window;

// Print help message and exits
static void
print_help ()
{
    printf("Usage: " PACKAGE_NAME " [options]\n"
"Options:\n"
"  -h, --help                  Show this message and exit.\n"
"  -v, --version               Show version information.\n"
"  -a ADDR, --address ADDR     Server address.\n\n"

"Report bugs to:\n"
"Antonio Serrano Hernandez (" PACKAGE_BUGREPORT ")\n"
    );
    exit (0);
}

// Print version message and exits
static void
print_version ()
{
    printf(PACKAGE_STRING "\n"
"Copyright (C) 2019 Antonio Serrano\n"
"This is free software; see the source for copying conditions.  There is NO\n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
    );
    exit (0);
}

// Parse the command line arguments
static void
parse_args (int argc, char **argv)
{
    struct option long_opts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"address", required_argument, 0, 'a'},
        {0, 0, 0, 0}
    };
    int o;

    do {
        o = getopt_long(argc, argv, OPTSTRING, long_opts, 0);
        switch (o) {
            case 'h':
                print_help ();
            case 'v':
                print_version ();
            case 'a':
                server_address = optarg;
                break;
            case '?':
                exit (1);
            default:
                break;
        }
    } while (o != -1);
    if (!server_address) {
        errx (1, "error: missing argument --address");
    }
}

static void
main_window_delete_clicked (GtkWidget *widget,
                            GdkEvent *event,
                            gpointer user_data)
{
    gtk_main_quit ();
}

static gboolean
main_window_key_pressed (GtkWidget *widget,
                         GdkEventKey *event,
                         gpointer data)
{
    if (event->keyval == FULLSCREEN_KEY) {
        if (main_window.is_fullscreen) {
            gtk_window_unfullscreen (GTK_WINDOW (main_window.window));
        } else {
            gtk_window_fullscreen (GTK_WINDOW (main_window.window));
        }
    }
    return FALSE;
}

static void
main_window_state_changed (GtkWidget *widget,
                           GdkEventWindowState *event,
                           gpointer data)
{
    main_window.is_fullscreen = (
        event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
}

static int
main_window_create ()
{
    main_window.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (main_window.window), 1280, 720);

    // Set styles
    GtkCssProvider *provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_path (provider, paths_get_styles(), NULL);
    gtk_style_context_add_provider_for_screen (gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Add the quit signal handler
    g_signal_connect (G_OBJECT (main_window.window), "delete-event",
        G_CALLBACK (main_window_delete_clicked), NULL);

    // Connect the signals to switch to fullscreen
    main_window.is_fullscreen = 0;
    g_signal_connect (G_OBJECT (main_window.window), "key-press-event",
        G_CALLBACK (main_window_key_pressed), NULL);
    g_signal_connect (G_OBJECT (main_window.window), "window-state-event",
        G_CALLBACK (main_window_state_changed), NULL);

    // Create the stack that contains all the views
    main_window.stack = gtk_stack_new ();
    gtk_container_add (GTK_CONTAINER (main_window.window), main_window.stack);
    gtk_widget_show_all (main_window.window);

    // Create the views
    if (splash_view_create ()) return -1;
    if (profiles_view_create ()) return -1;
    if (newprofile_view_create ()) return -1;
    if (medias_view_create ()) return -1;
    if (picture_view_create ()) return -1;

    // Populate the stack
    gtk_stack_add_named (
        GTK_STACK (main_window.stack), splash_view.box, "splash");
    gtk_stack_add_named (GTK_STACK (main_window.stack), profiles_view.box,
        "choose-profile");
    gtk_stack_add_named (GTK_STACK (main_window.stack), newprofile_view.box,
        "new-profile");
    gtk_stack_add_named (GTK_STACK (main_window.stack), medias_view.box,
        "medias");
    gtk_stack_add_named (GTK_STACK (main_window.stack), picture_view.box,
        "change-picture");

    // Show the first view
    main_window_change_view ("splash", NULL);
    return 0;
}

static void
main_window_destroy ()
{
    profiles_view_destroy ();
    medias_view_destroy ();
    gtk_widget_destroy (main_window.window);
}

static void
main_window_change_view_child (GtkWidget *child, void *data)
{
    main_window.last_view = gtk_stack_get_visible_child (
        GTK_STACK (main_window.stack));
    gtk_widget_show (child);
    gtk_stack_set_visible_child (GTK_STACK (main_window.stack), child);
    if (main_window.last_view) {
        gtk_widget_hide (main_window.last_view);
    }
}

// PUBLIC FUNCTIONS

void
main_window_change_view (const char *new_view, void *data)
{
    GtkWidget *new_child = gtk_stack_get_child_by_name (
        GTK_STACK (main_window.stack), new_view);
    main_window_change_view_child (new_child, data);
}

void
main_window_back ()
{
    main_window_change_view_child (main_window.last_view, NULL);
}

int
main (int argc, char **argv)
{
    parse_args (argc, argv);
    gtk_init (&argc, &argv);
    core_create (server_address);
    if (main_window_create ()) {
        return -1;
    }
    gtk_main ();
    core_destroy ();
    main_window_destroy ();
    return 0;
}

