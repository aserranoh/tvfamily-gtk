/*
widgets.h - Custom widgets.

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

#ifndef WIDGETS_H
#define WIDGETS_H

#include <gtk/gtk.h>

#include "coretypes.h"

#define MESSAGE_QUESTION    0
#define MESSAGE_ERROR       1

typedef struct MenuBar_s {
    GtkWidget *box;
    int height;
} MenuBar;

typedef struct ProfilesBox_s {
    GtkWidget *box;
    GtkWidget *hbox;
    int size;
    GtkCallback clicked_callback;
    GArray *buttons;
} ProfilesBox;

typedef struct ProfileMenu_s {
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *picture;
    GtkWidget *popover;
} ProfileMenu;

typedef struct CropImage_s {
    int w;
    int h;
    GdkPixbuf *pixbuf;
    GdkPixbuf *original_pixbuf;
    cairo_surface_t *mask;
    cairo_t *mc;
    GtkWidget *box;
    GtkWidget *drawing_area;
    int original_w;
    int original_h;
    int step_x;
    int step_y;
    int window_x;
    int window_y;
    int window_size;
    float scale;
    int origin_x;
    int origin_y;
} CropImage;

typedef struct anibutton_s {
    GdkPixbuf *pixbuf_black;
    GdkPixbuf *pixbuf_white;
    GtkWidget *icon;
} anibutton_t;

typedef struct {
    GtkWidget *box;
    size_t cols;
    GCallback click_callback;
    GtkWidget *grid;
    GPtrArray *medias;
    int poster_w;
    int poster_h;
} MediasBox;

typedef struct {
    GtkWidget *overlay;
    GtkWidget *button;
    GtkWidget *image;
    Media *media;
} MediaEntry;

typedef struct {
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *progress;
} ProgressDialog;

gboolean
exit_clicked (GtkWidget *widget, gpointer user_data);

int
menubar_create (MenuBar *m, int height);

void
menubar_add_front (MenuBar *m, GtkWidget *w);

void
menubar_add_back (MenuBar *m, GtkWidget *w);

/* Show a message box and return the response. */
int
message_new (GtkWidget *window, int type, const char *msg);

/* Create the box that contains the buttons for the profiles. */
void
profilesbox_create (ProfilesBox *p, int size, GtkCallback callback);

/* Set the list of profiles. Return 1 if the list has changed from the previous
   one, or 0 if not.
*/
gboolean
profilesbox_set (ProfilesBox *p, GPtrArray *profiles);

int
profilesbox_set_focus (ProfilesBox *p, int index);

void
profilesbox_set_picture (ProfilesBox *p,
                         const char *profile,
                         GdkPixbuf *picture);

void
profilesbox_clear (ProfilesBox *p);

void
profilemenu_create (ProfileMenu *m, int size, GtkCallback *callbacks);

void
profilemenu_set_name (ProfileMenu *m, const char *name);

void
profilemenu_set_picture (ProfileMenu *m, GdkPixbuf *pixbuf);

void
cropimage_create (CropImage *c, int w, int h);

int
cropimage_set_image (CropImage *c, const char *path);

GdkPixbuf *
cropimage_get_cropped_image (CropImage *c);

GtkWidget *
viewbutton (const char *label, GtkCallback callback, gpointer user_data);

GtkWidget *
anibutton (const char *images[],
           int size,
           GtkCallback callback,
           gpointer user_data,
           ...);

GtkWidget *
xlabel (const char *label, ...);

GtkWidget *
xbutton (const char *label,
         GtkCallback callback,
         gpointer user_data,
         ...);

void
mediasbox_create (MediasBox *m,
                  size_t num_cols,
                  GCallback media_clicked_callback);

void
mediasbox_set_poster_size (MediasBox *m, int w, int h);

gboolean
mediasbox_set_medias (MediasBox *box, GPtrArray *medias);

void
mediasbox_set_poster (MediasBox *box, Media *m, GdkPixbuf *poster);

void
mediasbox_select (MediasBox *m, int index);

void
progress_create (ProgressDialog *p);

void
progress_set_message (ProgressDialog *p, const char *message);

void
progress_set_progress (ProgressDialog *p, unsigned int progress);

void
progress_destroy (ProgressDialog *p);

#endif

