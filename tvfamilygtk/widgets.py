
'''widgets.py - Widgets used in the TVFamily views.

Copyright 2018 2019 Antonio Serrano Hernandez

This file is part of tvfamily-gtk.

tvfamily-gtk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

tvfamily-gtk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tvfamily; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.
'''

import cairo
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gdk, GdkPixbuf, GLib, Gtk, Pango
import os
import tempfile
import threading

from tvfamilygtk import paths as tfp

__author__ = 'Antonio Serrano Hernandez'
__copyright__ = 'Copyright (C) 2018 2019 Antonio Serrano Hernandez'
__version__ = '0.1'
__license__ = 'GPL'
__maintainer__ = 'Antonio Serrano Hernandez'
__email__ = 'toni.serranoh@gmail.com'
__status__ = 'Development'
__homepage__ = 'https://github.com/aserranoh/tvfamily-gtk'


class CropImage(Gtk.Box):
    '''A widget to show an image and crop it.'''

    MASK_COLOR = (1.0, 0.5, 0.31)
    ZOOM_STEP = 24
    INITIAL_WINDOW_SIZE = 256

    # Zoom icon constants
    ZOOM_IN_ICON_SIZE = (32, 32)
    ZOOM_OUT_ICON_SIZE = (24, 24)

    def __init__(self, size):
        Gtk.Box.__init__(self, orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.size = size
        self.pixbuf = None
        self.mask = cairo.ImageSurface(cairo.FORMAT_A1, *size)
        self.__build()
        self.__set_styles()

    def __build(self):
        '''Build the elements of this widget.'''
        # Create the drawing area
        self.vbox_darea = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        self.pack_start(self.vbox_darea, False, False, 0)
        self.drawing_area = Gtk.DrawingArea()
        self.drawing_area.set_size_request(*self.size)
        self.drawing_area.connect('draw', self.draw)
        self.vbox_darea.pack_start(self.drawing_area, True, False, 0)
        # Create the controlling buttons
        hbox_buttons = Gtk.Box(
            orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        self.pack_start(hbox_buttons, False, False, 0)
        left_button = ViewButton('◀', self.move, (0, -1))
        hbox_buttons.pack_start(left_button, True, True, 0)
        right_button = ViewButton('▶', self.move, (0, 1))
        hbox_buttons.pack_start(right_button, True, True, 0)
        up_button = ViewButton('▲', self.move, (1, -1))
        hbox_buttons.pack_start(up_button, True, True, 0)
        down_button = ViewButton('▼', self.move, (1, 1))
        hbox_buttons.pack_start(down_button, True, True, 0)
        zoom_icons = [tfp.get_image('zoom-black'), tfp.get_image('zoom-white')]
        zoom_in_button = ImageButton(zoom_icons, self.ZOOM_IN_ICON_SIZE,
            self.zoom_in_clicked, style='view-button')
        hbox_buttons.pack_start(zoom_in_button, True, True, 0)
        zoom_out_button = ImageButton(zoom_icons, self.ZOOM_OUT_ICON_SIZE,
            self.zoom_out_clicked, style='view-button')
        hbox_buttons.pack_start(zoom_out_button, True, True, 0)

    def __set_styles(self):
        '''Configure styles for this widget.'''
        self.get_style_context().add_class('crop-image')
        self.vbox_darea.set_name('profile-picture-editor')

    def move(self, widget, data):
        '''Move the window over the image.'''
        dim, direction = data
        self.window_pos[dim] += (self.step[0] * direction)
        self.adjust_window_pos()
        self.drawing_area.queue_draw()

    def zoom_in_clicked(self, widget):
        '''Button 'zoom in' of this widget has been clicked.'''
        self.window_size += self.ZOOM_STEP
        self.adjust_window_size()
        self.adjust_window_pos()
        self.drawing_area.queue_draw()

    def zoom_out_clicked(self, widget):
        '''Button 'zoom out' of this widget has been clicked.'''
        self.window_size -= self.ZOOM_STEP
        self.adjust_window_size()
        self.drawing_area.queue_draw()

    def adjust_window_pos(self):
        '''Make sure the whole window is over the image.'''
        wp = self.window_pos
        for i in range(2):
            if wp[i] < 0:
                wp[i] = 0
            elif wp[i] + self.window_size > self.original_size[i]:
                wp[i] = self.original_size[i] - self.window_size

    def adjust_window_size(self):
        '''Make sure the window is not bigger than the image.'''
        for i in range(2):
            if self.window_size > self.original_size[i]:
                self.window_size = self.original_size[i]
        if self.window_size < 1:
            self.window_size = 1

    def draw(self, widget, context):
        '''Draw the cropped image.'''
        if self.pixbuf is not None:
            # Calculate the position and size of the window at scale
            winpos = (int(self.window_pos[0]/self.scale),
                int(self.window_pos[1]/self.scale))
            winsize = int(self.window_size/self.scale)
            # Draw the window on the mask
            mc = cairo.Context(self.mask)
            mc.set_operator(cairo.OPERATOR_SOURCE)
            mc.set_source_rgba(0, 0, 0, 1)
            mc.rectangle(*self.origin, self.pixbuf.get_width(),
                self.pixbuf.get_height())
            mc.fill()
            mc.set_source_rgba(0, 0, 0, 0)
            mc.rectangle(self.origin[0] + winpos[0],
                self.origin[1] + winpos[1], winsize, winsize)
            mc.fill()
            # Paint the image
            Gdk.cairo_set_source_pixbuf(context, self.pixbuf, *self.origin)
            context.paint()
            # Paint a semitransparent color masked
            context.set_source_rgba(*self.MASK_COLOR, 0.5)
            context.mask_surface(self.mask)

    def set_image(self, path):
        '''Set the image to edit.'''
        if path is None:
            self.pixbuf = None
            return
        self.image_path = path
        # Keep the image's original size
        try:
            format, w, h = GdkPixbuf.Pixbuf.get_file_info(path)
            self.original_size = (w, h)
            self.step = self.original_size[0]/100, self.original_size[1]/100
            self.window_pos = [0, 0]
            self.window_size = self.INITIAL_WINDOW_SIZE
            self.adjust_window_size()
            # The image must be scaled keeping the aspect ratio
            original_ratio = w/h
            crop_ratio = self.size[0]/self.size[1]
            if original_ratio > crop_ratio:
                w = self.size[0]
                h = -1
                self.scale = self.original_size[0] / w
            else:
                w = -1
                h = self.size[1]
                self.scale = self.original_size[1] / h
            self.pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
                path, w, h, True)
            self.origin = ((self.size[0] - self.pixbuf.get_width())/2,
                (self.size[1] - self.pixbuf.get_height())/2)
            self.drawing_area.queue_draw()
        except GLib.Error:
            self.pixbuf = None
            raise

    def get_cropped_image(self):
        '''Return the cropped image.'''
        # Obtain the subimage from the original image
        pixbuf = GdkPixbuf.Pixbuf.new_from_file(self.image_path)
        return pixbuf.new_subpixbuf(
            *self.window_pos, self.window_size, self.window_size)


class EpisodesBox(Gtk.Box):
    '''Contains the list of episodes.'''

    def __init__(self, still_size):
        Gtk.Box.__init__(self, orientation=Gtk.Orientation.VERTICAL)
        self.still_size = still_size
        self.episodes = []

    def set_episodes(self, episodes):
        '''Set the episodes info.'''
        for c in self.get_children():
            self.remove(c)
            self.episodes = []
        for n, e in sorted(episodes.items()):
            b = EpisodeButton(e, self.still_size)
            self.pack_start(b, False, False, 0)
            self.episodes.append(b)
        self.show_all()

    def set_still(self, episode, still):
        '''Set the episode's still.'''
        self.episodes[episode - 1].set_still(still)

    def set_focus(self, episode):
        '''Set the focus in the given episode.'''
        self.episodes[episode - 1].set_focus()


class EpisodeButton(Gtk.Box):
    '''Contains the information of an episode.'''

    def __init__(self, info, still_size):
        Gtk.Box.__init__(self, orientation=Gtk.Orientation.HORIZONTAL)
        self.__build(info, still_size)

    def __build(self, info, still_size):
        '''Build the elements of this widget.'''
        # Create the still button
        self.still_button = Gtk.Button()
        self.pack_start(self.still_button, False, False, 0)
        # Create the image inside the button
        self.image = Gtk.Image()
        self.still_button.add(self.image)
        self.image.set_size_request(*still_size)
        # Create the box for the other information
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.pack_start(vbox, False, False, 0)
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        vbox.pack_start(hbox, False, False, 0)
        title = Label('S{}E{}: {}'.format(info['season'], info['episode'],
            info['title']))
        hbox.pack_start(title, False, False, 0)
        attrs = Label('{}|{}|{}'.format('date', 'duration', 'ratio'))
        hbox.pack_end(attrs, False, False, 0)
        plot = Label(info['plot'])
        plot.set_line_wrap(True)
        plot.set_justify(Gtk.Justification.FILL)
        vbox.pack_start(plot, False, False, 0)

    def set_still(self, still):
        '''Set this episode's still.'''
        self.image.set_from_pixbuf(still)

    def set_focus(self):
        '''Set the window focus over this episode.'''
        self.still_button.grab_focus()


class Image(object):
    '''An image that can be build from bytes or from a file.'''

    def __init__(self, source, size):
        if isinstance(source, bytes):
            self.pixbuf = self.get_pixbuf_from_bytes(source, size)
        else:
            keep_aspect_ratio = (size[0] < 0 or size[1] < 0)
            # The picture is the name of a file
            self.pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
                source, *size, keep_aspect_ratio)

    def get_pixbuf_from_bytes(self, stream, size):
        '''Return a pixbuf from a stream of bytes.'''
        fd, filename = tempfile.mkstemp()
        os.write(fd, stream)
        os.close(fd)
        keep_aspect_ratio = (size[0] < 0 or size[1] < 0)
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            filename, *size, keep_aspect_ratio)
        os.unlink(filename)
        return pixbuf


class ImageButton(Gtk.Button):
    '''A button with an image.'''

    def __init__(self, images, size, callback, style=None):
        Gtk.Button.__init__(self)
        self.__build(images, size, callback)
        self.__set_styles(style)

    def __build(self, images, size, callback):
        '''Build the elements of this widget.'''
        self.pixbuf_black = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            images[0], *size, True)
        self.pixbuf_white = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            images[1], *size, True)
        self.icon = Gtk.Image.new_from_pixbuf(self.pixbuf_black)
        self.add(self.icon)
        self.connect('focus-in-event', self.focus_in)
        self.connect('focus-out-event', self.focus_out)
        self.connect('enter-notify-event', self.focus_in)
        self.connect('leave-notify-event', self.focus_out)
        self.connect('clicked', callback)

    def __set_styles(self, style):
        '''Configure styles for this widget.'''
        if style is not None:
            self.get_style_context().add_class(style)

    def focus_in(self, widget, event):
        '''The button is focused.'''
        self.icon.set_from_pixbuf(self.pixbuf_white)

    def focus_out(self, widget, event):
        '''The button is not focused any more.'''
        self.icon.set_from_pixbuf(self.pixbuf_black)


class Label(Gtk.Label):
    '''A label with shadow.'''

    def __init__(self, text='', styles=[]):
        Gtk.Label.__init__(self, text)
        for s in styles:
            self.get_style_context().add_class(s)


class LabeledImageButton(Gtk.Button):
    '''A button with a label and an image.'''

    def __init__(self, label, size, callback=None):
        Gtk.Button.__init__(self)
        self.size = size
        self.__build(label, callback)
        self.__set_styles()

    def __build(self, label, callback):
        '''Build the elements of this widget.'''
        # Create a box inside the button that will contain the picture and the
        # label
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.add(box)
        # Add the image
        self.image = Gtk.Image()
        self.image.set_size_request(*self.size)
        box.pack_start(self.image, False, False, 0)
        # Add the label
        self.label = Gtk.Label(label)
        self.label.set_max_width_chars(1)
        self.label.set_hexpand(True)
        box.pack_start(self.label, True, False, 0)
        # Connect the widget to the callback
        if callback:
            self.connect('clicked', callback)

    def __set_styles(self):
        '''Configure the styles for this button.'''
        self.get_style_context().add_class('labeled-image-button')

    def set_image(self, pixbuf):
        '''Set the given pixbuf image to this button.'''
        self.image.set_from_pixbuf(pixbuf)


class MediasBox(Gtk.ScrolledWindow):
    '''Contains the list of medias to display.'''

    def __init__(self, cols, click_callback, focus_callback):
        Gtk.ScrolledWindow.__init__(self)
        self.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        self.cols = cols
        self.click_callback = click_callback
        self.focus_callback = focus_callback
        self.grid = Gtk.Grid()
        self.add_with_viewport(self.grid)

    def set_medias(self, medias):
        '''Set the list of medias.'''
        self.medias = {}
        # Remove the old entries
        for c in self.grid.get_children():
            self.grid.remove(c)
        # Add new entries
        for i, m in enumerate(medias):
            row = i // self.cols
            col = i % self.cols
            e = MediaEntry(m, self.click_callback, self.focus_callback)
            self.grid.attach(e, col, row, 1, 1)
            self.medias[m] = e
        self.show_all()

    def set_poster(self, media, poster):
        '''Set the poster for a given media.'''
        self.medias[media].set_image(poster)

    def do_show(self):
        '''Show everything except the horizontal scroll bar.'''
        Gtk.ScrolledWindow.do_show(self)
        self.get_vscrollbar().hide()

    def do_show_all(self):
        '''Show everything except the horizontal scroll bar.'''
        Gtk.ScrolledWindow.do_show_all(self)
        self.get_vscrollbar().hide()


class MenuBar(Gtk.Box):
    '''The menu bar that is shown in almost all the views.'''

    # MARGINS must coincide with the padding property in the styles.css
    MARGINS = 5

    def __init__(self, height, listener):
        Gtk.Box.__init__(self, orientation=Gtk.Orientation.HORIZONTAL)
        self.listener = listener
        self.height = height
        self.__build()
        self.__set_styles()

    def __build(self):
        '''Build the elements of this widget.'''
        # Add the TVFamily logo
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            tfp.get_logo(), -1, self.height - self.MARGINS*2, True)
        self.logo = Gtk.Image.new_from_pixbuf(pixbuf)
        self.pack_start(self.logo, False, False, 0)

    def __set_styles(self):
        '''Configure styles.'''
        self.get_style_context().add_class('bar')
        self.logo.get_style_context().add_class('logo')

    def add(self, front=[], back=[]):
        '''Add controls to the menu bar.'''
        for x in front:
            self.pack_start(x, False, False, 0)
        for x in back:
            self.pack_end(x, False, False, 0)
        self.show_all()


class MenuBarButton(Gtk.Button):
    '''A button in the menu bar.'''

    def __init__(self, label, callback, data=None):
        Gtk.Button.__init__(self, label)
        self.set_size_request(width=120, height=-1)
        self.get_style_context().add_class('bar-button')
        self.connect('clicked', callback, data)


class Message(Gtk.MessageDialog):
    '''A dialog with a personalized style.'''

    # Message types
    QUESTION = 0
    ERROR = 1

    PARAMS = [
        (Gtk.MessageType.QUESTION, Gtk.ButtonsType.YES_NO),
        (Gtk.MessageType.ERROR, Gtk.ButtonsType.OK),
    ]

    def __init__(self, window, type_, message):
        Gtk.MessageDialog.__init__(self, window, 0, self.PARAMS[type_][0],
            self.PARAMS[type_][1], message)
        self.get_style_context().add_class('message')
        self.set_border_width(30)
        self.get_action_area().set_spacing(40)
        self.set_decorated(False)


class PictureButton(Gtk.Button):
    '''A button with an image.'''

    def __init__(self, callback=None):
        Gtk.Button.__init__(self)
        #self.size = size
        self.__build(callback)
        self.__set_styles()

    def __build(self, callback):
        '''Build the elements of this widget.'''
        # Add the image
        self.image = Gtk.Image()
        self.add(self.image)
        # Connect the widget to the callback
        if callback:
            self.connect('clicked', callback)

    def __set_styles(self):
        '''Configure the styles for this button.'''
        self.get_style_context().add_class('picture-button')

    def set_image(self, pixbuf):
        '''Set the given pixbuf image to this button.'''
        self.image.set_from_pixbuf(pixbuf)


class MediaEntry(PictureButton):
    '''Represents a media of the list of medias, with poster and label.'''

    def __init__(self, media, click_callback, focus_callback):
        PictureButton.__init__(self, click_callback)
        self.media = media
        self.focus_callback = focus_callback
        self.connect('focus-in-event', self._focus_callback)

    def _focus_callback(self, widget, event):
        '''Call the focus callback.'''
        self.focus_callback(self, self.media)


class PixbufCache(object):
    '''A cache for pixbuf objects.'''

    def __init__(self):
        self.pixbufs = {}

    def get_pixbuf(self, path, size):
        '''Return a pixbuf from a given path.'''
        key = (os.path.basename(path), size[0])
        try:
            pixbuf = self.pixbufs[key]
        except KeyError:
            pixbuf = Image(path, size).pixbuf
            self.pixbufs[key] = pixbuf
        return pixbuf


class ProfileButton(LabeledImageButton):
    '''A button with a profile picture and a label.'''

    def __init__(self, name, size, callback=None):
        LabeledImageButton.__init__(self, name, size, callback)
        self.name = name
        self.__build()

    def __build(self):
        '''Build the elements of this widget.'''
        self.label.set_ellipsize(Pango.EllipsizeMode.END)

    def get_name(self):
        '''Return the profile's name.'''
        return self.name


class ProfileButtonsBox(Gtk.ScrolledWindow):
    '''Container for the profile buttons.'''

    def __init__(self, size, clicked_callback=None):
        Gtk.ScrolledWindow.__init__(self)
        self.size = size
        self.clicked_callback = clicked_callback
        self.profiles = {}
        self.__build()

    def __build(self):
        '''Build this widget.'''
        self.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.NEVER)
        self.hbox = Gtk.Box(
            orientation=Gtk.Orientation.HORIZONTAL, spacing=20)
        self.hbox.props.halign = Gtk.Align.CENTER
        self.add_with_viewport(self.hbox)

    def clear(self):
        '''Remove all the profiles buttons.'''
        for c in self.hbox.get_children():
            self.hbox.remove(c)
        self.profiles = {}

    def set_profile_focused(self, index):
        '''Set the focus to the given profile.'''
        children = self.hbox.get_children()
        if index < len(children):
            children[index].grab_focus()
        else:
            raise IndexError()

    def set_profiles(self, profiles):
        '''Set the list of profiles buttons.'''
        self.clear()
        for p in profiles:
            b = ProfileButton(p, self.size, self.clicked_callback)
            self.profiles[p] = b
            self.hbox.pack_start(b, False, False, 0)

    def do_show(self):
        '''Show everything except the horizontal scroll bar.'''
        Gtk.ScrolledWindow.do_show(self)
        self.get_hscrollbar().hide()

    def do_show_all(self):
        '''Show everything except the horizontal scroll bar.'''
        Gtk.ScrolledWindow.do_show_all(self)
        self.get_hscrollbar().hide()

    def set_profile_picture(self, profile, picture):
        '''Set the picture for a given profile.'''
        self.profiles[profile].set_image(picture)


class ProfileMenu(Gtk.Button):
    '''The profile menu.'''

    MAX_PROFILE_NAME_CHARS = 15

    def __init__(self, size, listener):
        Gtk.Button.__init__(self)
        self.__build(size, listener)
        self.__set_styles()

    def __build(self, size, listener):
        '''Build this widget.'''
        # Build the button with a label and an image
        hbox_button = Gtk.Box(
            orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        self.add(hbox_button)
        self.label = Gtk.Label()
        self.label.set_ellipsize(Pango.EllipsizeMode.END)
        self.label.set_max_width_chars(self.MAX_PROFILE_NAME_CHARS)
        hbox_button.pack_start(self.label, False, False, 0)
        # Add the image
        self.picture = Gtk.Image()
        self.picture.set_size_request(*size)
        hbox_button.pack_start(self.picture, True, True, 0)
        # Create the popover menu
        self.popover = Gtk.Popover()
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.popover.add(box)
        # Create the buttons that go into the popover
        self.change_picture = self.button(
            'Change picture', box, listener.change_profile_picture)
        self.settings = self.button('Settings', box, listener.settings)
        self.change_profile = self.button(
            'Change profile', box, listener.change_profile)
        self.delete_profile = self.button(
            'Delete profile', box, listener.delete_profile)
        self.quit = self.button('Quit', box, listener.exit_clicked)
        # Connect actions
        self.connect('clicked', self.clicked)

    def button(self, label, box, callback):
        '''Create a button for this menu.'''
        b = Gtk.Button()
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        hbox.pack_start(Gtk.Label(label), False, False, 0)
        b.connect('clicked', callback)
        b.add(hbox)
        box.pack_start(b, False, False, 0)
        return b

    def __set_styles(self):
        '''Configure styles for this widget.'''
        self.get_style_context().add_class('bar-button')

    def set_name(self, name):
        '''Set the profile's name.'''
        self.label.set_text(name)

    def set_picture(self, pixbuf):
        '''Set the profile's picture.'''
        self.picture.set_from_pixbuf(pixbuf)

    def clicked(self, widget):
        '''The menu button has been clicked. Show the menu.'''
        self.popover.set_relative_to(self)
        self.popover.show_all()
        self.popover.popup()


class SeasonsButtonsBox(Gtk.Grid):
    '''Contains the buttons to select season and episode.'''

    SPACING = 20

    def __init__(self, buttons_per_row, callback):
        Gtk.Grid.__init__(self)
        self.set_row_spacing(self.SPACING)
        self.set_column_spacing(self.SPACING)
        self.buttons_per_row = buttons_per_row
        self.callback = callback
        self.current_season = None
        self.seasons_buttons = []

    def set_seasons(self, seasons):
        '''Set the seasons buttons.'''
        for c in self.get_children():
            self.remove(c)
            self.seasons_buttons = []
        for i in range(seasons):
            b = ViewButton(
                '{} {}'.format('Season', i + 1), self.season_clicked, i + 1)
            b.get_style_context().add_class('season-button')
            row = i / self.buttons_per_row
            col = i % self.buttons_per_row
            self.seasons_buttons.append(b)
            self.attach(b, col, row, 1, 1)
        self.show_all()

    def season_clicked(self, widget, season):
        '''A season button has been clicked.'''
        if self.current_season is not None:
            self.current_season.get_style_context().remove_class(
                'current-season')
        widget.get_style_context().add_class('current-season')
        self.current_season = widget
        self.callback(season)

    def select_season(self, season):
        '''Select a season.'''
        self.season_clicked(self.seasons_buttons[season - 1], season)


class ServerRequest(object):
    '''Launch a server request in a parallel thread.'''

    def __init__(self, method, args, callback, timeout=0):
        self.method = method
        self.args = args
        self.callback = callback
        self.timeout = timeout
        self.cancelled = False
        self.finished = False
        self.error = None

    def run(self):
        '''Run the request.'''
        threading.Thread(target=self.thread_main).start()

    def thread_main(self):
        '''Thread's body.'''
        try:
            self.result = self.method(*self.args)
            self.error = None
        except Exception as e:
            self.error = e
        GLib.idle_add(self.loop_callback)

    def loop_callback(self):
        '''Request done, synchronized with the GLib main loop.'''
        if not self.cancelled:
            self.callback(self)
            if self.error and self.timeout:
                GLib.timeout_add_seconds(self.timeout, self.run)
            else:
                self.finished = True
        else:
            self.finished = True

    def cancel(self):
        '''Cancel this request.'''
        self.cancelled = True


class ServerRequestList(object):
    '''Manages a list of server request.'''

    def __init__(self):
        self.requests = set()

    def add(self, method, args, callback, timeout=0):
        '''Add a server request and execute it. Return the new request.'''
        self.cleanup()
        request = ServerRequest(method, args, callback, timeout)
        self.requests.add(request)
        request.run()
        return request

    def cancel(self, request):
        '''Cancel a given request.'''
        self.cleanup()
        request.cancel()
        self.requests.remove(request)

    def cancel_all(self):
        '''Cancel all server requests.'''
        for r in self.requests:
            r.cancel()
        self.requests = set()

    def cleanup(self):
        '''Cleanup this container of finished requests.'''
        l = list(self.requests)
        for r in l:
            if r.finished:
                self.requests.remove(r)


class View(Gtk.Box):
    '''Base class for all the views.'''

    def __init__(self, window, core=None):
        Gtk.Box.__init__(self, orientation=Gtk.Orientation.VERTICAL)
        self.window = window
        self.core = core
        self.visible = False

    def exit_clicked(self, widget=None):
        '''Ask for confirmation to exit the application.'''
        response = message(self.window, Message.QUESTION,
            'Are you sure you want to exit?')
        if response == Gtk.ResponseType.YES:
            Gtk.main_quit()
            return False
        return True

    def shown(self):
        '''Executed when this view is shown.'''
        pass


class MenuBarView(View):
    '''A view with a menu bar.'''

    def __init__(self, window, core):
        View.__init__(self, window, core)
        bar_height = window.get_allocated_height() / 10
        self.bar = MenuBar(height=bar_height, listener=self)
        self.pack_start(self.bar, False, False, 0)
        self.contents_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.pack_start(self.contents_box, True, True, 0)


class ViewButton(Gtk.Button):
    '''A button in any view.'''

    def __init__(self, label='', callback=None, data=None):
        Gtk.Button.__init__(self, label)
        self.get_style_context().add_class('view-button')
        if callback:
            self.connect('clicked', callback, data)


# Convenience functions

def message(window, type, msg):
    '''Show a message dialog.'''
    m = Message(window, type, msg)
    response = m.run()
    m.destroy()
    return response

