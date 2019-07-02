/*
profilesview.cpp - The view to choose a profile.

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

/*
static gboolean
profiles_view_update_picture (PictureRequest *r)
{
    GdkPixbuf *p = NULL;
    GInputStream *gis;

    if (r->error) {
        p = gdk_pixbuf_new_from_file_at_scale (paths_get_default_picture(),
            PROFILE_PICTURE_SIZE, PROFILE_PICTURE_SIZE, TRUE, NULL);
    } else {
        if (r->picture->len > 0) {
            gis = g_memory_input_stream_new_from_data (
                r->picture->data, r->picture->len, NULL);
            p = gdk_pixbuf_new_from_stream_at_scale (gis,
                PROFILE_PICTURE_SIZE, PROFILE_PICTURE_SIZE, TRUE, NULL, NULL);
            if (!p) {
                warnx ("profiles_view_update_picture: cannot load profile "
                    "picture");
                p = gdk_pixbuf_new_from_file_at_scale (
                    paths_get_default_picture(), PROFILE_PICTURE_SIZE,
                    PROFILE_PICTURE_SIZE, TRUE, NULL);
            }
            g_object_unref (gis);
        } else {
            p = gdk_pixbuf_new_from_file_at_scale (paths_get_default_picture(),
                PROFILE_PICTURE_SIZE, PROFILE_PICTURE_SIZE, TRUE, NULL);
        }
    }
    const char *profile = (const char *)r->data;
    profilesbox_set_picture (
        &profiles_view.profiles_box, profile, p);
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
profiles_view_leave (const char *next_view)
{
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
}*/

#include <glibmm/main.h>
#include <gtkmm/messagedialog.h>

#include "paths.h"
#include "profilesview.h"
#include "question.h"

ProfilesView::ProfilesView (ViewControllerInterface& controller):
    BarView (controller),
    exit_button (Paths::get_image ("off-black"),
        Paths::get_image ("off-white"),
        get_bar ().get_height () - 2*BAR_ICON_MARGIN,
        sigc::mem_fun (*this, &ProfilesView::on_exit),
        {"bar-element", "bar-button", "bar-icon-button"}),
    contents_box (Gtk::ORIENTATION_VERTICAL, 20),
    profiles_box (PROFILE_PICTURE_SIZE, *this),
    label (""),
    stack (),
    new_profile_button_box (Gtk::ORIENTATION_HORIZONTAL),
    new_profile_button ("New profile"),
    profile_got_focus (false),
    button_got_focus (false)
{
    // Populate the menu bar
    get_bar ().add_back (exit_button.get_button ());

    // Complete the label
    label.get_style_context ()->add_class ("view-label");

    // Create the contents
    auto& main_box = get_box ();
    main_box.pack_start (contents_box, true, true);
    contents_box.set_valign (Gtk::ALIGN_CENTER);
    contents_box.pack_start (stack, true, true);
    stack.add (profiles_box.get_box (), "profiles");
    stack.add (label, "label");

    // Create an horizontal box that contains the 'New profile button', because
    // if it is put in the vertical box it expands horizontally, and we don't
    // want that
    contents_box.pack_start (new_profile_button_box, false, false);

    // Create the New profile button
    new_profile_button.get_style_context ()->add_class ("view-button");
    new_profile_button.signal_clicked ().connect (
        sigc::mem_fun (*this, &ProfilesView::on_new_profile_clicked));
    new_profile_button_box.pack_start (new_profile_button, true, false);

    // Show all elements and then hide the main box
    main_box.show_all ();
}

ProfilesView::~ProfilesView ()
{}

void ProfilesView::show ()
{
    get_controller ().get_core ().request_profiles (*this);
}

void ProfilesView::profiles_received (std::unique_ptr<ProfilesResult>& result)
{
    this->profiles = std::move(result);
    Glib::signal_idle ().connect (
        sigc::mem_fun (*this, &ProfilesView::on_profiles_received));
}

void ProfilesView::profile_clicked (const std::string& profile)
{
}

void ProfilesView::on_new_profile_clicked ()
{

}

bool ProfilesView::on_profiles_received ()
{
    auto retry = false;

    // Exit if this view is not visible
    if (get_controller ().get_current_view () != &get_box ()) {
        return false;
    }
    // Check the profiles received
    if (profiles->get_error ()) {
        // Some error ocurred, show the message
        show_label ("Cannot get list of profiles");
        retry = true;
    } else {
        if (not profiles->size ()) {
            // No profiles available
            show_label ("No profiles available");
            retry = true;
        } else {
            // Set the profiles list
            profiles_box.set (profiles->get_profiles ());
            // Request the profiles pictures
            for (auto& p: profiles->get_profiles ()) {
                get_controller ().get_core ().request_profile_picture (
                    p, *this);
            }
            stack.set_visible_child ("profiles-box");
        }
    }
    set_default_focus ();

    if (retry) {
        // Request again the list of profiles after a given timeout
        Glib::signal_timeout ().connect (
            sigc::mem_fun (*this, &ProfilesView::on_timeout),
            QUERY_PROFILES_TIMEOUT);
    }
    return false;
}

void ProfilesView::on_exit ()
{
    auto& w = get_controller ().get_window ();
    auto response = Question (w, "Are you sure you want to exit?").run ();
    if (response == Gtk::RESPONSE_YES) {
        get_controller ().exit ();
    }
}

bool ProfilesView::on_timeout ()
{
    get_controller ().get_core ().request_profiles (*this);
    return false;
}

void ProfilesView::show_label (const std::string& text)
{
    label.set_text (text);
    stack.set_visible_child (label);
}

void ProfilesView::set_default_focus ()
{
    // If the focus hasn't been given to a profile yet, make a profile have
    // the focus, if any. If not, give the focus to the create_profile_button.
    if (not profile_got_focus) {
        if (profiles_box.set_focus (0)) {
            profile_got_focus = true;
        } else {
            if (not button_got_focus) {
                new_profile_button.grab_focus ();
                button_got_focus = true;
            }
        }
    }
}

