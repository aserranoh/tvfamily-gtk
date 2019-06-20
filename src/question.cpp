
#include "question.h"

Question::Question (Gtk::Window& parent, const std::string& message):
    dialog (parent, message, false, Gtk::MESSAGE_QUESTION,
        Gtk::BUTTONS_YES_NO, true)
{
    dialog.get_style_context ()->add_class ("message");
    dialog.set_border_width (30);
    dialog.get_action_area ()->set_spacing (40);
    dialog.set_decorated (false);
}

Question::~Question ()
{}

int Question::run ()
{
    auto response = dialog.run ();
    return response;
}

