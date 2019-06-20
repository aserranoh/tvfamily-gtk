
#include "barview.h"

const float BarView::BAR_RATIO = 0.1;

BarView::BarView (ViewControllerInterface& controller):
    View (controller),
    bar (Gdk::Screen::get_default ()->get_height () * BAR_RATIO)
{
    // Add the bar to the main box
    get_box ().pack_start (bar.get_box (), false, false);
}

BarView::~BarView ()
{}

MenuBar& BarView::get_bar ()
{
    return bar;
}

