/*
profilesbox.cpp - Holds a list of ProfileButton.

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

#include <gtkmm/scrollbar.h>

#include "profilesbox.h"

ProfilesBox::ProfilesBox (int size, ProfileButtonListener& listener):
    size (size), listener (listener), box (),
    profile_buttons_box (Gtk::ORIENTATION_HORIZONTAL, 20), buttons ()
{
    box.signal_show ().connect (sigc::mem_fun (*this, &ProfilesBox::on_show));
    box.set_policy (Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER);
    profile_buttons_box.set_halign (Gtk::ALIGN_CENTER);
    box.add (profile_buttons_box);
}

ProfilesBox::~ProfilesBox ()
{}

void ProfilesBox::on_show ()
{
    box.get_hscrollbar ()->hide ();
}

void ProfilesBox::set (const std::vector<std::string>& profiles)
{
    // Remove the absent buttons
    for (auto it = buttons.begin(); it != buttons.end();) {
        bool found = false;
        for (auto& p: profiles) {
            if ((*it)->get_name () == p) {
                found = true;
                break;
            }
        }
        if (not found) {
            // The current button is not among the new profiles. Remove it.
            profile_buttons_box.remove ((*it)->get_button ());
            it = buttons.erase (it);
        } else {
            it++;
        }
    }

    // Add the new buttons
    for (auto& p: profiles) {
        bool found = false;
        for (auto& b: buttons) {
            if (b->get_name () == p) {
                found = true;
                break;
            }
        }
        if (not found) {
            // The current profile is not among the current buttons. Add it.
            auto b = std::make_unique<ProfileButton> (p, size, listener);
            profile_buttons_box.pack_start (b->get_button (), false, false);
            buttons.push_back (std::move (b));
        }
    }
}

bool ProfilesBox::set_focus (int index)
{
    if (index >= buttons.size ()) {
        return false;
    } else {
        buttons[index]->get_button ().grab_focus ();
    }
    return true;
}

