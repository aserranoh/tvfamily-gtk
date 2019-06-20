/*
profilesview.h - The view to choose a profile.

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

#ifndef PROFILESVIEW_H
#define PROFILESVIEW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/stack.h>
#include <gtkmm/window.h>
#include <string>
#include <vector>

#include "animatedbutton.h"
#include "barview.h"
#include "profilepicturelistener.h"
#include "profilesbox.h"
#include "profileslistener.h"

class ProfilesView: public BarView, ProfilesListener, ProfilePictureListener {

    private:

        // Constants
        static const int BAR_ICON_MARGIN = 5;
        static const int PROFILE_PICTURE_SIZE = 128;
        static const int QUERY_PROFILES_TIMEOUT = 5;

        // Exit application button
        AnimatedButton exit_button;

        // Box that contains the contents of this view
        Gtk::Box contents_box;

        // Control that holds the profiles buttons
        ProfilesBox profiles_box;

        // Label to show a message of no profiles available
        Gtk::Label label;

        // Stack to switch between the label and the profiles box
        Gtk::Stack stack;

        // Box that contains the new profile button (for proper alignment)
        Gtk::Box new_profile_button_box;

        // Button to create a new profile
        Gtk::Button new_profile_button;

        // List of profiles to show
        std::unique_ptr<ProfilesResult> profiles;

        // True if a profile button got the focus
        bool profile_got_focus;

        // True if the new profile button got the focus
        bool button_got_focus;

    public:

        ProfilesView (ViewControllerInterface& controller);
        ~ProfilesView ();

        // Show this view
        void show ();

        // Implementation of the interface ProfilesListener.
        void profiles_received (std::unique_ptr<ProfilesResult>& result);

    private:

        // Executed when the exit button is clicked
        void on_exit ();

        // Executed when a profile is clicked
        void on_profile_clicked ();

        // Executed when the new profile button is clicked
        void on_new_profile_clicked ();

        // Executed when the list of profiles is received
        bool on_profiles_received ();

        // Executed when the timeout is expired
        bool on_timeout ();

        // Put a text in the info label
        void show_label (const std::string& text);

        // Set the focus to the right element
        void set_default_focus ();

};

#endif

