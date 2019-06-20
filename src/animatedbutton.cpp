
#include "animatedbutton.h"

AnimatedButton::AnimatedButton (const std::string& black_icon,
                                const std::string& white_icon,
                                int size,
                                sigc::slot<void> callback,
                                const std::vector<std::string>& css_classes):
    button (),
    pixbuf_black (Gdk::Pixbuf::create_from_file (black_icon, -1, size, true)),
    pixbuf_white (Gdk::Pixbuf::create_from_file (white_icon, -1, size, true)),
    image (pixbuf_black)
{
    // Remove the border of the button
    button.set_border_width (0);

    // And add the image inside the button
    button.add (image);

    // Connect signals
    button.signal_focus_in_event ().connect (
        [this] (GdkEventFocus*) {return on_focus_in ();});
    button.signal_focus_out_event ().connect (
        [this] (GdkEventFocus*) {return on_focus_out ();});
    button.signal_enter_notify_event ().connect (
        [this] (GdkEventCrossing*) {return on_focus_in ();});
    button.signal_leave_notify_event ().connect (
        [this] (GdkEventCrossing*) {return on_focus_out ();});
    button.signal_clicked ().connect (callback);

    // Set styles
    auto context = button.get_style_context ();
    for (auto& cls: css_classes) {
        context->add_class (cls);
    }
}

AnimatedButton::~AnimatedButton ()
{}

Gtk::Button& AnimatedButton::get_button ()
{
    return button;
}

bool AnimatedButton::on_focus_in ()
{
    image.set (pixbuf_white);
    return false;
}

bool AnimatedButton::on_focus_out ()
{
    image.set (pixbuf_black);
    return false;
}

