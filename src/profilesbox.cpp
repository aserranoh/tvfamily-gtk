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

ProfilesBox::ProfilesBox (int size, sigc::slot<void> callback):
    size (size), callback (callback), box (),
    profile_buttons_box (Gtk::ORIENTATION_HORIZONTAL, 20)
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
    bool found = false;

    // Compare the new list against the current one
    if (buttons.size () == profiles.size ()) {
        found = true;
        for (auto& b: buttons) {
            found = false;
            for (auto& p: profiles) {
                if (b.get_name () == p) {
                    found = true;
                    break;
                }
            }
        }
    }

    // If necessary, set the new profiles list
    if (not found) {
        
    }
    
    // Clear the previous buttons
    profilesbox_clear (p);

    // Set the new buttons
    p->buttons = g_array_sized_new (FALSE, FALSE, sizeof (b), profiles->len);
    g_array_set_clear_func (p->buttons, (GDestroyNotify)profilebutton_destroy);
    for (int i = 0; i < profiles->len; i++) {
        profilebutton_create (&b,
            (const char *)g_ptr_array_index (profiles, i), p->size,
            p->clicked_callback);
        gtk_box_pack_start (GTK_BOX (p->hbox), b.button, FALSE, FALSE, 0);
        g_array_append_val (p->buttons, b);
    }
    return TRUE;
}
}

bool ProfilesBox::set_focus (int index)
{
    if (index >= buttons.size ()) {
        return false;
    } else {
        buttons[index].get_button ().grab_focus ();
    }
    return true;
}

