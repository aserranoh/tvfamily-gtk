/*
profilebutton.cpp - A button with a profile picture and name.

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

#include "profilebutton.h"

ProfileButton::ProfileButton (const std::string& name,
                              int size,
                              ProfileButtonListener& listener):
    listener (listener), box (Gtk::ORIENTATION_VERTICAL, 5), button (),
    image (), label (name)
{
    button.add (box);
    image.set_size_request (size, size);
    box.pack_start (image, false, false);
    label.set_max_width_chars (1);
    label.set_hexpand (true);
    label.set_ellipsize (Pango::ELLIPSIZE_END);
    box.pack_start (label, true, false);
    button.signal_clicked ().connect (
        sigc::mem_fun (*this, &ProfileButton::on_button_clicked));
    button.get_style_context ()->add_class ("labeled-image-button");
}

ProfileButton::~ProfileButton ()
{}

void ProfileButton::on_button_clicked ()
{
    listener.profile_clicked (get_name ());
}

