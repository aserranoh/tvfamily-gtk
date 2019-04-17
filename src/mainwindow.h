/*
mainwindow.h - The interface with the main window.

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk/gtk.h>

typedef struct _MainWindow {
    GtkWidget *window;
    GtkWidget *stack;
    GtkWidget *last_view;
    int is_fullscreen;
} MainWindow;

extern MainWindow main_window;

int
main_window_get_width ();

int
main_window_get_height ();

void
main_window_change_view (const char *new_view, void *data);

void
main_window_back ();

void
main_window_destroy ();

#endif

