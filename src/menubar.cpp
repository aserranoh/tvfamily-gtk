
#include "menubar.h"
#include "paths.h"

MenuBar::MenuBar (int height):
    box (Gtk::ORIENTATION_HORIZONTAL), logo (), height (height)
{
    // Create the pixbuf with the logo image
    auto pixbuf = Gdk::Pixbuf::create_from_file (
        Paths::get_logo (), -1, height - LOGO_MARGIN * 2, true);

    // Create the image from the pixbuf
    logo.set (pixbuf);
    box.pack_start (logo, false, false);

    // Configure styles
    box.get_style_context ()->add_class ("bar");
    logo.get_style_context ()->add_class ("logo");
}

MenuBar::~MenuBar ()
{}

Gtk::Box& MenuBar::get_box ()
{
    return box;
}

int MenuBar::get_height () const
{
    return height;
}

void MenuBar::add_back (Gtk::Widget &widget)
{
    box.pack_end (widget, false, false);
}

