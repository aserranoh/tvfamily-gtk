/*
widgets.c - Custom widgets.

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

#include <err.h>
#include <gtk/gtk.h>

#include "paths.h"
#include "widgets.h"

#define BAR_LOGO_MARGIN 5

#define MAX_PROFILE_NAME_CHARS  15

#define ZOOM_IN_ICON_SIZE   32
#define ZOOM_OUT_ICON_SIZE  24

#define CROP_IMAGE_INITIAL_WINDOW_SIZE  256
#define CROP_IMAGE_MASK_COLOR_R         1.0
#define CROP_IMAGE_MASK_COLOR_G         0.5
#define CROP_IMAGE_MASK_COLOR_B         0.31
#define CROP_IMAGE_ZOOM_STEP            24

// TYPES

/* Represents a profile button. */
typedef struct ProfileButton_s {
    GtkWidget *button;
    GtkWidget *image;
    char *name;
} ProfileButton;

// PRIVATE FUNCTIONS

/* Create a new button for the profiles box. */
static void
profilebutton_create (ProfileButton *b,
                      const char *profile,
                      int size,
                      GtkCallback callback)
{
    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *label = gtk_label_new (profile);

    b->name = g_strdup (profile);
    b->button = gtk_button_new ();
    b->image = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (b->button), box);
    gtk_widget_set_size_request (b->image, size, size);
    gtk_box_pack_start (GTK_BOX (box), b->image, FALSE, FALSE, 0);
    gtk_label_set_max_width_chars (GTK_LABEL (label), 1);
    gtk_widget_set_hexpand (label, TRUE);
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, FALSE, 0);
    g_signal_connect (b->button, "clicked", G_CALLBACK (callback), b->name);
    GtkStyleContext *context = gtk_widget_get_style_context (b->button);
    gtk_style_context_add_class (context, "labeled-image-button");
}

/* Change the picture of a ProfileButton. */
static void
profilebutton_set_picture (ProfileButton *b, GdkPixbuf *picture)
{
    gtk_image_set_from_pixbuf (GTK_IMAGE (b->image), picture);
    g_object_unref (picture);
}

/* Destroy the contents of a ProfileButton. */
static void
profilebutton_destroy (ProfileButton *b)
{
    gtk_widget_destroy (b->button);
    g_free (b->name);
}

/* The profiles box is show, hide the scrollbar. */
static gboolean
profilesbox_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    GtkWidget *hs = gtk_scrolled_window_get_hscrollbar (
        GTK_SCROLLED_WINDOW (widget));
    gtk_widget_hide (hs);
    return FALSE;
}

// PUBLIC FUNCTIONS

gboolean
exit_clicked (GtkWidget *widget, gpointer user_data)
{
    GtkWidget *window = (GtkWidget *)user_data;
    int response = message_new (
        window, MESSAGE_QUESTION, "Are you sure you want to exit?");
    if (response == GTK_RESPONSE_YES) {
        gtk_main_quit ();
        return FALSE;
    } else {
        return TRUE;
    }
}

int
menubar_create (MenuBar *m, int height)
{
    GdkPixbuf *pixbuf;
    GtkWidget *logo;

    // Create the main box
    m->height = height;
    m->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    // Load the pixbuf
    pixbuf = gdk_pixbuf_new_from_file_at_scale (paths_get_logo (),
        -1, height - BAR_LOGO_MARGIN * 2, TRUE, 0);
    if (!pixbuf) {
        warnx ("error: cannot load TVFamily logo");
        return -1;
    }

    // Create the image
    logo = gtk_image_new_from_pixbuf (pixbuf);
    gtk_box_pack_start (GTK_BOX (m->box), logo, FALSE, FALSE, 0);

    // Configure styles
    GtkStyleContext *context = gtk_widget_get_style_context (m->box);
    gtk_style_context_add_class (context, "bar");
    context = gtk_widget_get_style_context (logo);
    gtk_style_context_add_class (context, "logo");

    // Unref the logo pixbuf (the reference has been copied by the image)
    g_object_unref (pixbuf);
    return 0;
}

void
menubar_add_front (MenuBar *m, GtkWidget *w)
{
    gtk_box_pack_start (GTK_BOX (m->box), w, FALSE, FALSE, 0);
}

void
menubar_add_back (MenuBar *m, GtkWidget *w)
{
    gtk_box_pack_end (GTK_BOX (m->box), w, FALSE, FALSE, 0);
}

int
message_new (GtkWidget *window, int type, const char *msg)
{
    GtkMessageType msg_type;
    GtkButtonsType buttons;

    if (type == MESSAGE_QUESTION) {
        msg_type = GTK_MESSAGE_QUESTION;
        buttons = GTK_BUTTONS_YES_NO;
    } else {
        msg_type = GTK_MESSAGE_ERROR;
        buttons = GTK_BUTTONS_OK;
    }
    GtkWidget *m = gtk_message_dialog_new (
        GTK_WINDOW (window), 0, msg_type, buttons, "%s", msg);
    GtkStyleContext *context = gtk_widget_get_style_context (m);
    gtk_style_context_add_class (context, "message");
    gtk_container_set_border_width (GTK_CONTAINER (m), 30);
    gtk_box_set_spacing (
        GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (m))), 40);
    gtk_window_set_decorated (GTK_WINDOW (m), FALSE);
    int response = gtk_dialog_run (GTK_DIALOG (m));
    gtk_widget_destroy (m);
    return response;
}

void
profilesbox_create (ProfilesBox *p, int size, GtkCallback callback)
{
    p->size = size;
    p->clicked_callback = callback;
    p->buttons = NULL;

    // Create the scrolled window
    p->box = gtk_scrolled_window_new (NULL, NULL);
    g_signal_connect (p->box, "show", G_CALLBACK (profilesbox_show), NULL);
    gtk_scrolled_window_set_policy (
        GTK_SCROLLED_WINDOW (p->box), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);

    // Create the box inside the scrolled window
    p->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign (p->hbox, GTK_ALIGN_CENTER);
    gtk_container_add (GTK_CONTAINER (p->box), p->hbox);
}

gboolean
profilesbox_set (ProfilesBox *p, GPtrArray *profiles)
{
    ProfileButton b;

    // Compare the new list against the current one
    if (p->buttons && profiles->len == p->buttons->len) {
        gboolean found = TRUE;
        for (int i = 0; i < p->buttons->len && found; i++) {
            const char *name = g_array_index (
                p->buttons, ProfileButton, i).name;
            found = FALSE;
            for (int j = 0; j < profiles->len && !found; j++) {
                if (strcmp (g_ptr_array_index (profiles, j), name)) {
                    found = TRUE;
                }
            }
        }
        if (found) return FALSE;
    }

    // Clear the previous buttons
    profilesbox_clear (p);

    // Set the new buttons
    p->buttons = g_array_sized_new (FALSE, FALSE, sizeof (b), profiles->len);
    g_array_set_clear_func (p->buttons, (GDestroyNotify)profilebutton_destroy);
    for (int i = 0; i < profiles->len; i++) {
        profilebutton_create (
            &b, g_ptr_array_index (profiles, i), p->size, p->clicked_callback);
        gtk_box_pack_start (GTK_BOX (p->hbox), b.button, FALSE, FALSE, 0);
        g_array_append_val (p->buttons, b);
    }
    return TRUE;
}

int
profilesbox_set_focus (ProfilesBox *p, int index)
{
    GList *l = gtk_container_get_children (GTK_CONTAINER (p->hbox));
    if (index >= g_list_length (l)) {
        return -1;
    } else {
        gtk_widget_grab_focus (GTK_WIDGET (g_list_nth_data (l, index)));
    }
    return 0;
}

void
profilesbox_set_picture (ProfilesBox *p,
                         const char *profile,
                         GdkPixbuf *picture)
{
    ProfileButton *b;

    for (int i = 0; i < p->buttons->len; i++) {
        b = &g_array_index (p->buttons, ProfileButton, i);
        if (strcmp (b->name, profile) == 0) {
            profilebutton_set_picture (b, picture);
            break;
        }
    }
}

void
profilesbox_clear (ProfilesBox *p)
{
    if (p->buttons) {
        g_array_free (p->buttons, TRUE);
        p->buttons = NULL;
    }
}

static GtkWidget *
profilemenu_new_button (ProfileMenu *m, const char *label, GtkWidget *box)
{
    GtkWidget *b = gtk_button_new ();
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (
        GTK_BOX (hbox), gtk_label_new (label), FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (b), hbox);
    gtk_box_pack_start (GTK_BOX (box), b, FALSE, FALSE, 0);
    return b;
}

static void
profilemenu_clicked (GtkButton *button, gpointer user_data)
{
    ProfileMenu *m = (ProfileMenu *)user_data;
    gtk_widget_show_all (m->popover);
    gtk_popover_popup (GTK_POPOVER (m->popover));
}

void
profilemenu_create (ProfileMenu *m, int size, GtkCallback *callbacks)
{
    GtkWidget *change_picture, *settings, *change_profile, *delete_profile;
    GtkWidget *quit;

    // Build the button with a label and an image
    m->button = gtk_button_new ();
    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add (GTK_CONTAINER (m->button), hbox);
    m->label = gtk_label_new ("");
    gtk_label_set_ellipsize (GTK_LABEL (m->label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars (
        GTK_LABEL (m->label), MAX_PROFILE_NAME_CHARS);
    gtk_box_pack_start (GTK_BOX (hbox), m->label, FALSE, FALSE, 0);

    // Add the image
    m->picture = gtk_image_new ();
    gtk_widget_set_size_request (m->picture, size, size);
    gtk_box_pack_start (GTK_BOX (hbox), m->picture, TRUE, TRUE, 0);

    // Create the popover menu
    m->popover = gtk_popover_new (m->button);
    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (m->popover), box);

    // Create the buttons that go into the popover
    change_picture = profilemenu_new_button (m, "Change picture", box);
    settings = profilemenu_new_button (m, "Settings", box);
    change_profile = profilemenu_new_button (m, "Change profile", box);
    delete_profile = profilemenu_new_button (m, "Delete profile", box);
    quit = profilemenu_new_button (m, "Quit", box);

    // Connect actions
    g_signal_connect (
        m->button, "clicked", G_CALLBACK (profilemenu_clicked), m);
    g_signal_connect (
        change_picture, "clicked", G_CALLBACK (callbacks[0]), NULL);
    g_signal_connect (settings, "clicked", G_CALLBACK (callbacks[1]), NULL);
    g_signal_connect (
        change_profile, "clicked", G_CALLBACK (callbacks[2]), NULL);
    g_signal_connect (
        delete_profile, "clicked", G_CALLBACK (callbacks[3]), NULL);
    g_signal_connect (quit, "clicked", G_CALLBACK (callbacks[4]), NULL);

    // Set styles
    GtkStyleContext *context = gtk_widget_get_style_context (m->button);
    gtk_style_context_add_class (context, "bar-button");
}

void
profilemenu_set_name (ProfileMenu *m, const char *name)
{
    gtk_label_set_text (GTK_LABEL (m->label), name);
}

void
profilemenu_set_picture (ProfileMenu *m, GdkPixbuf *pixbuf)
{
    gtk_image_set_from_pixbuf (GTK_IMAGE (m->picture), pixbuf);
}

gboolean
cropimage_draw (GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    if (c->pixbuf) {
        // Calculate the position and size of the window at scale
        int winpos_x = c->window_x / c->scale;
        int winpos_y = c->window_y / c->scale;
        int winsize = c->window_size / c->scale;

        // Draw the window on the mask
        cairo_set_operator (c->mc, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgba (c->mc, 0, 0, 0, 1);
        cairo_rectangle (c->mc, c->origin_x, c->origin_y,
            gdk_pixbuf_get_width (c->pixbuf),
            gdk_pixbuf_get_height(c->pixbuf));
        cairo_fill (c->mc);
        cairo_set_source_rgba (c->mc, 0, 0, 0, 0);
        cairo_rectangle (c->mc, c->origin_x + winpos_x, c->origin_y + winpos_y,
            winsize, winsize);
        cairo_fill (c->mc);

        // Paint the image
        gdk_cairo_set_source_pixbuf (cr, c->pixbuf, c->origin_x, c->origin_y);
        cairo_paint (cr);

        // Paint a semitransparent color masked
        cairo_set_source_rgba (cr, CROP_IMAGE_MASK_COLOR_R,
            CROP_IMAGE_MASK_COLOR_G, CROP_IMAGE_MASK_COLOR_B, 0.5);
        cairo_mask_surface (cr, c->mask, 0, 0);
    }
    return FALSE;
}

static void
cropimage_adjust_window_pos (CropImage *c)
{
    // Make sure the whole window is over the image
    if (c->window_x < 0) {
        c->window_x = 0;
    }
    else if (c->window_x + c->window_size > c->original_w) {
        c->window_x = c->original_w - c->window_size;
    }
    if (c->window_y < 0) {
        c->window_y = 0;
    }
    else if (c->window_y + c->window_size > c->original_h) {
        c->window_y = c->original_h - c->window_size;
    }
}

static void
cropimage_adjust_window_size (CropImage *c)
{
    if (c->window_size > c->original_w) c->window_size = c->original_w;
    if (c->window_size > c->original_h) c->window_size = c->original_h;
    if (c->window_size < 1) c->window_size = 1;
}

static void
cropimage_movel (GtkWidget *widget, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    c->window_x -= c->step_x;
    cropimage_adjust_window_pos (c);
    gtk_widget_queue_draw (c->drawing_area);
}

static void
cropimage_mover (GtkWidget *widget, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    c->window_x += c->step_x;
    cropimage_adjust_window_pos (c);
    gtk_widget_queue_draw (c->drawing_area);
}

static void
cropimage_moveu (GtkWidget *widget, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    c->window_y -= c->step_y;
    cropimage_adjust_window_pos (c);
    gtk_widget_queue_draw (c->drawing_area);
}

static void
cropimage_moved (GtkWidget *widget, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    c->window_y += c->step_y;
    cropimage_adjust_window_pos (c);
    gtk_widget_queue_draw (c->drawing_area);
}

static void
cropimage_zoom_in (GtkWidget *widget, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    c->window_size += CROP_IMAGE_ZOOM_STEP;
    cropimage_adjust_window_size (c);
    cropimage_adjust_window_pos (c);
    gtk_widget_queue_draw (c->drawing_area);
}

static void
cropimage_zoom_out (GtkWidget *widget, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    c->window_size -= CROP_IMAGE_ZOOM_STEP;
    cropimage_adjust_window_size (c);
    gtk_widget_queue_draw (c->drawing_area);
}

void
cropimage_create (CropImage *c, int w, int h)
{
    c->w = w;
    c->h = h;
    c->image_path = NULL;
    c->pixbuf = NULL;
    c->mask = cairo_image_surface_create (CAIRO_FORMAT_A1, w, h);
    c->mc = cairo_create (c->mask);

    // Create the drawing area
    c->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *vbox_darea = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (
        GTK_BOX (c->box), vbox_darea, FALSE, FALSE, 0);
    c->drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_size_request (c->drawing_area, w, h);
    g_signal_connect (
        c->drawing_area, "draw", G_CALLBACK (cropimage_draw), c);
    gtk_box_pack_start (
        GTK_BOX (vbox_darea), c->drawing_area, TRUE, FALSE, 0);

    // Create the controlling buttons
    GtkWidget *hbox_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start (
        GTK_BOX (c->box), hbox_buttons, FALSE, FALSE, 0);
    GtkWidget *lbutton = viewbutton ("◀", cropimage_movel, c);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), lbutton, TRUE, TRUE, 0);
    GtkWidget *rbutton = viewbutton ("▶", cropimage_mover, c);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), rbutton, TRUE, TRUE, 0);
    GtkWidget *ubutton = viewbutton ("▲", cropimage_moveu, c);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), ubutton, TRUE, TRUE, 0);
    GtkWidget *dbutton = viewbutton ("▼", cropimage_moved, c);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), dbutton, TRUE, TRUE, 0);
    const char *zoom_icons[] = {
        paths_get_image ("zoom-black"),
        paths_get_image ("zoom-white")
    };
    GtkWidget *zoom_in_button = anibutton (
        zoom_icons, ZOOM_IN_ICON_SIZE, cropimage_zoom_in, c, "view-button");
    gtk_box_pack_start (GTK_BOX (hbox_buttons), zoom_in_button, TRUE, TRUE, 0);
    GtkWidget *zoom_out_button = anibutton (
        zoom_icons, ZOOM_OUT_ICON_SIZE, cropimage_zoom_out, c, "view-button");
    gtk_box_pack_start (
        GTK_BOX (hbox_buttons), zoom_out_button, TRUE, TRUE, 0);
    g_free ((char *)zoom_icons[0]);
    g_free ((char *)zoom_icons[1]);

    // Configure styles
    GtkStyleContext *context = gtk_widget_get_style_context (c->box);
    gtk_style_context_add_class (context, "crop-image");
    gtk_widget_set_name (vbox_darea, "profile-picture-editor");
}

int
cropimage_set_image (CropImage *c, const char *path)
{
    // Destroy the previous pixbuf, if any
    if (c->pixbuf) {
        g_object_unref (c->pixbuf);
        g_free (c->image_path);
        c->pixbuf = NULL;
        c->image_path = NULL;
    }

    if (!path) {
        return 0;
    } else {
        c->image_path = g_strdup (path);
    }

    // Keep the image's original size
    // TODO: refresh the image if error
    if (!gdk_pixbuf_get_file_info (path, &c->original_w, &c->original_h)) {
        c->pixbuf = NULL;
        return -1;
    }
    c->step_x = c->original_w/100;
    c->step_y = c->original_h/100;
    c->window_x = 0;
    c->window_y = 0;
    c->window_size = CROP_IMAGE_INITIAL_WINDOW_SIZE;
    cropimage_adjust_window_size (c);

    // The image must be scaled keeping the aspect ratio
    float original_ratio = c->original_w/c->original_h;
    float crop_ratio = c->w/c->h;
    int w, h;
    if (original_ratio > crop_ratio) {
        w = c->w;
        h = -1;
        c->scale = (float)c->original_w / (float)w;
    } else {
        w = -1;
        h = c->h;
        c->scale = (float)c->original_h / (float)h;
    }
    c->pixbuf = gdk_pixbuf_new_from_file_at_scale (path, w, h, TRUE, 0);
    if (!c->pixbuf) {
        c->pixbuf = NULL;
        return -1;
    }
    c->origin_x = (c->w - gdk_pixbuf_get_width (c->pixbuf)) / 2;
    c->origin_y = (c->h - gdk_pixbuf_get_height (c->pixbuf)) / 2;
    gtk_widget_queue_draw (c->drawing_area);
    return 0;
}

int
cropimage_get_cropped_image (CropImage *c, GdkPixbuf **subp)
{
    int err = 0;

    *subp = NULL;
    if (c->pixbuf) {
        GdkPixbuf *p = gdk_pixbuf_new_from_file (c->image_path, NULL);
        if (!p) {
            err = -1;
        } else {
            *subp = gdk_pixbuf_new_subpixbuf (p, c->window_x, c->window_y,
                c->window_size, c->window_size);
            g_object_unref (p);
        }
    }
    return err;
}

GtkWidget *
viewbutton (const char *label, GtkCallback callback, gpointer user_data)
{
    GtkWidget *b = gtk_button_new_with_label (label);
    GtkStyleContext *context = gtk_widget_get_style_context (b);
    gtk_style_context_add_class (context, "view-button");
    if (callback) {
        g_signal_connect (b, "clicked", G_CALLBACK (callback), user_data);
    }
    return b;
}

static gboolean
anibutton_focus_in (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    anibutton_t *a = (anibutton_t *)user_data;
    gtk_image_set_from_pixbuf (GTK_IMAGE (a->icon), a->pixbuf_white);
    return FALSE;
}

static gboolean
anibutton_focus_out (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    anibutton_t *a = (anibutton_t *)user_data;
    gtk_image_set_from_pixbuf (GTK_IMAGE (a->icon), a->pixbuf_black);
    return FALSE;
}

static gboolean
anibutton_destroy (GtkWidget *widget, gpointer user_data)
{
    anibutton_t *a = (anibutton_t *)user_data;
    g_object_unref (a->pixbuf_black);
    g_object_unref (a->pixbuf_white);
    g_free (a);
}

GtkWidget *
anibutton (const char *images[],
           int size,
           GtkCallback callback,
           gpointer user_data,
           const char *style)
{
    anibutton_t *a = g_new (anibutton_t, 1);
    GtkWidget *b = gtk_button_new ();
    gtk_container_set_border_width (GTK_CONTAINER (b), 0);
    a->pixbuf_black = gdk_pixbuf_new_from_file_at_scale (
        images[0], -1, size, TRUE, 0);
    if (!a->pixbuf_black) {
        warnx ("cannot load image %s", images[0]);
    }
    a->pixbuf_white = gdk_pixbuf_new_from_file_at_scale (
        images[1], size, -1, TRUE, 0);
    if (!a->pixbuf_white) {
        warnx ("cannot load image %s", images[1]);
    }
    a->icon = gtk_image_new_from_pixbuf (a->pixbuf_black);
    gtk_container_add (GTK_CONTAINER (b), a->icon);
    g_signal_connect (b, "focus-in-event", G_CALLBACK (anibutton_focus_in), a);
    g_signal_connect (
        b, "focus-out-event", G_CALLBACK (anibutton_focus_out), a);
    g_signal_connect (
        b, "enter-notify-event", G_CALLBACK (anibutton_focus_in), a);
    g_signal_connect (
        b, "leave-notify-event", G_CALLBACK (anibutton_focus_out), a);
    if (callback) {
        g_signal_connect (b, "clicked", G_CALLBACK (callback), user_data);
    }
    g_signal_connect (b, "destroy", G_CALLBACK (anibutton_destroy), a);
    GtkStyleContext *context = gtk_widget_get_style_context (b);
    gtk_style_context_add_class (context, style);
    return b;
}

GtkWidget *
xlabel (const char *label, ...)
{
    GtkWidget *l = gtk_label_new (label);
    GtkStyleContext *context = gtk_widget_get_style_context (l);
    const char *s;
    va_list va;
    va_start (va, label);
    while ((s = va_arg (va, const char *))) {
        gtk_style_context_add_class (context, s);
    }
    return l;
}

GtkWidget *
xbutton (const char *label,
         GtkCallback callback,
         gpointer user_data,
         const char *style)
{
    GtkWidget *b = gtk_button_new_with_label (label);
    g_signal_connect (b, "clicked", G_CALLBACK (callback), user_data);
    GtkStyleContext *context = gtk_widget_get_style_context (b);
    gtk_style_context_add_class (context, style);
    return b;
}

void
mediasbox_create (MediasBox *m,
                  size_t num_cols,
                  GtkCallback media_clicked_callback,
                  GtkCallback media_focused_callback)
{
    m->box = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (
        GTK_SCROLLED_WINDOW (m->box), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    m->cols = num_cols;
    m->click_callback = media_clicked_callback;
    m->focus_callback = media_focused_callback;
    m->grid = gtk_grid_new ();
    gtk_container_add (GTK_CONTAINER (m->box), m->grid);
    m->medias = g_ptr_array_new ();
}

void
mediasbox_set (MediasBox *box, GArray *medias)
{
    int i, row, col;
    GtkWidget e;
    Media *m;

    // Remove the old entries
    for (i = 0; i < box->medias->len; i++) {
        gtk_container_remove (
            GTK_CONTAINER (box->grid), g_ptr_array_index (box->medias, i));
    }
    g_ptr_array_remove_range (box->medias, 0, box->medias->len);

    // Add new entries
    for (i = 0; i < medias->len; i++) {
        m = &g_array_index (medias, Media, i);
        row = i / box->cols;
        col = i % box->cols;
        e = mediaentry_new (
            m, box->poster_size, box->click_callback, box->focus_callback);
        gtk_grid_attach (GTK_GRID (box->grid), col, row, 1, 1);
        g_ptr_array_add (box->medias, e);
    }
    gtk_widget_show_all (box->box);
}

void
mediasbox_select (MediasBox *m, int index)
{

}


