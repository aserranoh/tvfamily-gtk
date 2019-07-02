/*
mainwindow.cpp - The interface with the main window.

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

#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>

#include "viewcontroller.h"
#include "paths.h"

ViewController::ViewController (Glib::RefPtr<Gtk::Application>& app,
                                const std::string& server_address):
    app (app), window (), stack (),
    splash_view (*this),
    profiles_view (*this),
    newprofile_view (*this),
    medias_view (*this),
    picture_view (*this),
    mediainfo_view (*this),
    player_view (*this),
    views_map ({{"splash", &splash_view}, {"choose-profile", &profiles_view},
        {"new-profile", &newprofile_view}, {"medias", &medias_view},
        {"change-picture", &picture_view}, {"media-info", &mediainfo_view},
        {"player", &player_view}}),
    core (server_address)
{
    window.set_default_size (1280, 720);

    // Set styles
    auto provider = Gtk::CssProvider::create ();
    provider->load_from_path (Paths::get_styles ());
    auto screen = Gdk::Screen::get_default ();
    Gtk::StyleContext::add_provider_for_screen (
        screen, provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Connect the signals to switch to fullscreen
    is_fullscreen = false;
    window.signal_key_press_event ().connect (
        sigc::mem_fun (*this, &ViewController::on_key_press));
    window.signal_window_state_event ().connect (
        sigc::mem_fun (*this, &ViewController::on_window_state));
    window.add_events (Gdk::KEY_PRESS_MASK);

    // Add the stack to the window
    window.add (stack);
    window.show_all ();

    // Populate the stack
    for (auto x: views_map) {
        stack.add (x.second->get_box (), x.first);
    }

    // Show the first view
    switch_view ("splash");
}

ViewController::~ViewController ()
{}

void ViewController::switch_view (const std::string& new_view)
{
    // Update the member last_view
    last_view = stack.get_visible_child_name ();

    // Show the new child
    stack.set_visible_child (new_view);
    views_map[new_view]->show ();
}

void ViewController::switch_view (const std::string& new_view,
                                  const ViewSwitchData& data)
{
    // Update the member last_view
    last_view = stack.get_visible_child_name ();

    // Show the new child
    stack.set_visible_child (new_view);
    views_map[new_view]->show (data);
}

void ViewController::back ()
{
    switch_view (last_view);
}

Gtk::Widget* ViewController::get_current_view ()
{
    return stack.get_visible_child ();
}

void ViewController::exit ()
{
    app->quit ();
}

bool ViewController::on_key_press (GdkEventKey* event)
{
    if (event->keyval == FULLSCREEN_KEY) {
        if (is_fullscreen) {
            window.unfullscreen ();
        } else {
            window.fullscreen ();
        }
    }
    return false;
}


bool ViewController::on_window_state (GdkEventWindowState* event)
{
    is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    return false;
}

