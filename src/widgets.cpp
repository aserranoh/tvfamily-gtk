
#define MAX_PROFILE_NAME_CHARS  15

#define ZOOM_IN_ICON_SIZE   32
#define ZOOM_OUT_ICON_SIZE  24

#define CROP_IMAGE_INITIAL_WINDOW_SIZE  256
#define CROP_IMAGE_MASK_COLOR_R         1.0
#define CROP_IMAGE_MASK_COLOR_G         0.5
#define CROP_IMAGE_MASK_COLOR_B         0.31
#define CROP_IMAGE_ZOOM_STEP            24

/* Change the picture of a ProfileButton. */
static void
profilebutton_set_picture (ProfileButton *b, GdkPixbuf *picture)
{
    gtk_image_set_from_pixbuf (GTK_IMAGE (b->image), picture);
    g_object_unref (picture);
}

// PUBLIC FUNCTIONS

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
    gtk_style_context_add_class (context, "bar-element");
    gtk_style_context_add_class (context, "bar-button");
    gtk_style_context_add_class (context, "bar-menu");
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
    if (pixbuf) {
        g_object_unref (pixbuf);
    }
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
        cairo_set_source_rgba (c->mc, 0, 0, 0, 0);
        cairo_paint (c->mc);
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

static void
cropimage_destroy (GtkWidget *widget, gpointer user_data)
{
    CropImage *c = (CropImage *)user_data;
    if (c->pixbuf) {
        g_object_unref (c->pixbuf);
    }
    if (c->original_pixbuf) {
        g_object_unref (c->original_pixbuf);
    }
    cairo_surface_destroy (c->mask);
    cairo_destroy (c->mc);
}

void
cropimage_create (CropImage *c, int w, int h)
{
    c->w = w;
    c->h = h;
    c->pixbuf = NULL;
    c->original_pixbuf = NULL;
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
    GtkWidget *zoom_in_button = anibutton (zoom_icons, ZOOM_IN_ICON_SIZE,
        cropimage_zoom_in, c, "view-button", NULL);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), zoom_in_button, TRUE, TRUE, 0);
    GtkWidget *zoom_out_button = anibutton (zoom_icons, ZOOM_OUT_ICON_SIZE,
        cropimage_zoom_out, c, "view-button", NULL);
    gtk_box_pack_start (
        GTK_BOX (hbox_buttons), zoom_out_button, TRUE, TRUE, 0);
    g_free ((char *)zoom_icons[0]);
    g_free ((char *)zoom_icons[1]);

    // Configure styles
    GtkStyleContext *context = gtk_widget_get_style_context (c->box);
    gtk_style_context_add_class (context, "crop-image");
    gtk_widget_set_name (vbox_darea, "profile-picture-editor");

    // Connect the destroy signal
    g_signal_connect (c->box, "destroy", G_CALLBACK (cropimage_destroy), c);
}

int
cropimage_set_image (CropImage *c, const char *path)
{
    // Destroy the previous pixbuf, if any
    if (c->pixbuf) {
        g_object_unref (c->pixbuf);
        g_object_unref (c->original_pixbuf);
        c->pixbuf = NULL;
        c->original_pixbuf = NULL;
    }

    if (!path) {
        return 0;
    }

    // Keep the original image
    // TODO: refresh the image if error
    c->original_pixbuf = gdk_pixbuf_new_from_file (path, NULL);
    if (!c->original_pixbuf) {
        c->pixbuf = NULL;
        return -1;
    }
    c->original_w = gdk_pixbuf_get_width (c->original_pixbuf);
    c->original_h = gdk_pixbuf_get_height (c->original_pixbuf);
    c->step_x = c->original_w/100;
    c->step_y = c->original_h/100;
    c->window_x = 0;
    c->window_y = 0;
    c->window_size = CROP_IMAGE_INITIAL_WINDOW_SIZE;
    cropimage_adjust_window_size (c);

    // The image must be scaled keeping the aspect ratio
    float original_ratio = (float)c->original_w/(float)c->original_h;
    float crop_ratio = (float)c->w/(float)c->h;
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

GdkPixbuf *
cropimage_get_cropped_image (CropImage *c)
{
    GdkPixbuf *subp = NULL;

    if (c->pixbuf) {
        subp = gdk_pixbuf_new_subpixbuf (c->original_pixbuf,
            c->window_x, c->window_y, c->window_size, c->window_size);
    }
    return subp;
}

/*GtkWidget *
viewbutton (const char *label, GtkCallback callback, gpointer user_data)
{
    GtkWidget *b = gtk_button_new_with_label (label);
    GtkStyleContext *context = gtk_widget_get_style_context (b);
    gtk_style_context_add_class (context, "view-button");
    if (callback) {
        g_signal_connect (b, "clicked", G_CALLBACK (callback), user_data);
    }
    return b;
}*/

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
         ...)
{
    GtkWidget *b = gtk_button_new_with_label (label);
    g_signal_connect (b, "clicked", G_CALLBACK (callback), user_data);
    GtkStyleContext *context = gtk_widget_get_style_context (b);
    const char *s;
    va_list va;
    va_start (va, user_data);
    while ((s = va_arg (va, const char *))) {
        gtk_style_context_add_class (context, s);
    }
    return b;
}

static void
mediaentry_destroyed (GtkWidget *widget, gpointer user_data) {
    MediaEntry *e = (MediaEntry *)user_data;
    media_destroy (e->media);
    g_free (e);
}

static MediaEntry *
mediaentry_new (Media *m,
                int poster_w,
                int poster_h,
                GCallback click_callback)
{
    MediaEntry *e = g_new (MediaEntry, 1);
    e->overlay = gtk_overlay_new ();
    e->button = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (e->overlay), e->button);

    // Title label
    char *s = media_to_string (m);
    GtkWidget *title_label = xlabel (
        s, "media-label", NULL);
    g_free (s);
    gtk_widget_set_halign (title_label, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (title_label, GTK_ALIGN_END);
    gtk_label_set_line_wrap (GTK_LABEL (title_label), TRUE);
    gtk_label_set_justify (GTK_LABEL (title_label), GTK_JUSTIFY_CENTER);
    gtk_overlay_add_overlay (GTK_OVERLAY (e->overlay), title_label);

    // Rating label
    if (g_strcmp0 (m->rating, "") != 0) {
        GtkWidget *rating_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        const char *starfile = paths_get_image ("star");
        GtkWidget *img = gtk_image_new_from_file (starfile);
        g_free ((char *)starfile);
        GtkWidget *rating_label = xlabel (
            m->rating, "view-label", "rating", NULL);
        gtk_box_pack_start (GTK_BOX (rating_box), img, FALSE, FALSE, 0);
        gtk_box_pack_start (
            GTK_BOX (rating_box), rating_label, FALSE, FALSE, 0);
        gtk_widget_set_halign (rating_box, GTK_ALIGN_END);
        gtk_widget_set_valign (rating_box, GTK_ALIGN_START);
        gtk_style_context_add_class (
            gtk_widget_get_style_context (rating_box), "rating-box");
        gtk_overlay_add_overlay (GTK_OVERLAY (e->overlay), rating_box);
    }

    e->image = gtk_image_new ();
    e->media = m;
    gtk_widget_set_size_request (e->image, poster_w, poster_h);
    gtk_container_add (GTK_CONTAINER (e->button), e->image);
    g_signal_connect (e->button, "clicked", click_callback, e);
    gtk_style_context_add_class (
        gtk_widget_get_style_context (e->button), "picture-button");
    g_signal_connect (
        e->overlay, "destroy", G_CALLBACK (mediaentry_destroyed), e);
    return e;
}

static void
mediaentry_set_image (MediaEntry *e, GdkPixbuf *pixbuf)
{
    gtk_image_set_from_pixbuf (GTK_IMAGE (e->image), pixbuf);
}

static void
mediaentry_destroy (MediaEntry *e)
{
    gtk_widget_destroy (e->overlay);
}

/* The medias box is show, hide the scrollbar. */
static gboolean
mediasbox_show (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    GtkWidget *vs = gtk_scrolled_window_get_vscrollbar (
        GTK_SCROLLED_WINDOW (widget));
    gtk_widget_hide (vs);
    return FALSE;
}

static void
mediasbox_destroyed (GtkWidget *widget, gpointer user_data)
{
    MediasBox *box = (MediasBox *)user_data;
    g_ptr_array_free (box->medias, TRUE);
}

void
mediasbox_create (MediasBox *m,
                  size_t num_cols,
                  GCallback media_clicked_callback)
{
    m->box = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (
        GTK_SCROLLED_WINDOW (m->box), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    m->cols = num_cols;
    m->click_callback = media_clicked_callback;
    m->grid = gtk_grid_new ();
    gtk_container_add (GTK_CONTAINER (m->box), m->grid);
    m->medias = g_ptr_array_new_with_free_func (
        (GDestroyNotify)mediaentry_destroy);
    g_signal_connect (m->box, "show", G_CALLBACK (mediasbox_show), NULL);
    g_signal_connect (m->box, "destroy", G_CALLBACK (mediasbox_destroyed), m);
}

void
mediasbox_set_poster_size (MediasBox *m, int w, int h)
{
    m->poster_w = w;
    m->poster_h = h;
}

static gboolean
mediasbox_equals (MediasBox *box, GPtrArray *medias)
{
    int i, j, found = 0;
    Media *m;

    for (i = 0; i < medias->len; i++) {
        for (j = 0; j < box->medias->len; j++) {
            m = ((MediaEntry *)g_ptr_array_index (box->medias, j))->media;
            if (media_equal ((Media *)g_ptr_array_index (medias, i), m)) {
                found++;
            }
        }
    }
    return found == medias->len;
}

gboolean
mediasbox_set_medias (MediasBox *box, GPtrArray *medias)
{
    int i, row, col;
    MediaEntry *e;
    Media *m;

    if (!mediasbox_equals (box, medias)) {
        // Remove the old entries
        g_ptr_array_remove_range (box->medias, 0, box->medias->len);

        // Add new entries
        for (i = 0; i < medias->len; i++) {
            m = (Media *)g_ptr_array_index (medias, i);
            row = i / box->cols;
            col = i % box->cols;
            e = mediaentry_new (
                m, box->poster_w, box->poster_h, box->click_callback);
            g_ptr_array_add (box->medias, e);
            gtk_grid_attach (GTK_GRID (box->grid), e->overlay, col, row, 1, 1);
        }
        gtk_widget_show_all (box->box);
        return TRUE;
    }
    return FALSE;
}

void
mediasbox_set_poster (MediasBox *box, Media *m, GdkPixbuf *poster)
{
    MediaEntry *e;

    for (int i = 0; i < box->medias->len; i++) {
        e = (MediaEntry *)g_ptr_array_index (box->medias, i);
        if (g_strcmp0 (e->media->title_id, m->title_id) == 0) {
            mediaentry_set_image (e, poster);
        }
    }
    g_object_unref (poster);
}

void
mediasbox_select (MediasBox *m, int index)
{
    MediaEntry *e;
    if (0 <= index && index < m->medias->len) {
        e = (MediaEntry *)g_ptr_array_index (m->medias, index);
        gtk_widget_grab_focus (e->button);
    }
}

void
progress_create (ProgressDialog *p)
{
    // Create the dialog
    p->dialog = gtk_dialog_new ();
    gtk_window_set_modal (GTK_WINDOW (p->dialog), TRUE);
    GtkWidget *box = gtk_dialog_get_content_area (GTK_DIALOG (p->dialog));

    // Create the label
    p->label = gtk_label_new ("");
    gtk_box_pack_start (GTK_BOX (box), p->label, FALSE, FALSE, 0);

    // Create the progress bar
    p->progress = gtk_progress_bar_new ();
    gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (p->progress), TRUE);
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR (p->progress), NULL);
    gtk_box_pack_start (GTK_BOX (box), p->progress, FALSE, FALSE, 0);

    gtk_widget_show_all (p->dialog);
}

void
progress_set_message (ProgressDialog *p, const char *message)
{
    gtk_label_set_text (GTK_LABEL (p->label), message);
}

void
progress_set_progress (ProgressDialog *p, unsigned int progress)
{
    gtk_progress_bar_set_fraction (
        GTK_PROGRESS_BAR (p->progress), (double)progress/100.0);
}

void
progress_destroy (ProgressDialog *p)
{
    gtk_widget_destroy (p->dialog);
}

