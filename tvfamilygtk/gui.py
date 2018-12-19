
'''gui.py - Windows layer.

Copyright 2018 Antonio Serrano Hernandez

tvfamily is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

tvfamily is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tvfamily; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.
'''

# TODO: signals handling
# TODO: final static path(invalid address)

import cairo
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GLib, Gdk, GdkPixbuf
import os
import time

import tvfamilygtk.core

__author__ = 'Antonio Serrano Hernandez'
__copyright__ = 'Copyright (C) 2018 Antonio Serrano Hernandez'
__version__ = '0.1'
__license__ = 'GPL'
__maintainer__ = 'Antonio Serrano Hernandez'
__email__ = 'toni.serranoh@gmail.com'
__status__ = 'Development'
__homepage__ = 'https://github.com/aserranoh/tvfamily-gtk'

STATIC_PATH = 'data'
LOGO_PATH = os.path.join(STATIC_PATH, 'tvfamily.svg')
LOGO_WIDTH = 256
STYLES_PATH = os.path.join(STATIC_PATH, 'styles.css')
KEY_FULLSCREEN = Gdk.KEY_F11
TMP_DIR = '/tmp'
GET_PROFILES_RETRY_SECONDS = 5

# Exit icon constants
EXIT_ICON_BLACK = os.path.join(STATIC_PATH, 'off-black.svg')
EXIT_ICON_WHITE = os.path.join(STATIC_PATH, 'off-white.svg')
EXIT_ICON_WIDTH = 64

# Splash screen constants
SPLASH_ICON_WIDTH = 800
SPLASH_SECONDS = 0

# Profile images constants
PROFILE_PICTURE_SIZE = (128, 128)
PROFILE_DEFAULT_PATH = os.path.join(STATIC_PATH, 'profile-default.svg')
PROFILE_PICTURE_SIZE_SMALL = (60, 60)

# Convenience functions

def message(window, type, msg):
    '''Show a message dialog.'''
    m = Message(window, type, msg)
    response = m.run()
    m.destroy()
    return response

def new_exit_button(parent):
    '''Create a new exit button.'''
    exit_button = ImageButton([EXIT_ICON_BLACK, EXIT_ICON_WHITE],
        (EXIT_ICON_WIDTH, -1), parent.exit_clicked, style='bar-button')
    return exit_button

def save_picture_to_file(picture, filename):
    '''Save a picture to a file.'''
    if picture is not None:
        try:
            # Save the received image into a file
            with open(filename, 'wb') as f:
                f.write(picture)
        except IOError:
            pass

def load_picture_from_file(filename, size):
    '''Load a picture from a file at size and return a pixbuf.'''
    # Create a pixbuf with the given image
    try:
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(filename, *size, True)
    except GLib.Error:
        # Fallback to the default image
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            PROFILE_DEFAULT_PATH, *size, True)
    return pixbuf


class GUIManager(object):
    '''The root object of the GUI layer.'''

    def __init__(self, server_address):
        # Create an instance of the application core
        self.core = tvfamilygtk.core.Core(server_address)
        # Create an instance of the main window
        self.main_window = MainWindow(self.core)
        # Configure styles
        self.__set_styles()

    def __set_styles(self):
        '''Set the styles for the widgets.'''
        css_provider = Gtk.CssProvider()
        with open(STYLES_PATH, 'rb') as f:
            css_provider.load_from_data(f.read())
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(),
            css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    def run(self):
        Gtk.main()


class MainWindow(Gtk.Window):
    '''The application window.'''

    def __init__(self, core):
        Gtk.Window.__init__(self, title='TVFamily GTK')
        self.core = core
        # Build the widget
        self.__build_widget()

    def __build_widget(self):
        '''Build this widget.'''
        # Add the quit signal handler
        self.connect('delete-event', self._delete_clicked)
        self.set_size_request(800, 600)
        # Connect the signals to switch to fullscreen
        self.is_fullscreen = False
        self.connect('key-press-event', self._key_pressed)
        self.connect('window-state-event', self._state_changed)
        # Build the stack that contains all the views
        self.stack = Gtk.Stack()
        self.stack.add_titled(SplashView(self), 'splash', 'splash screen')
        self.stack.add_titled(ChooseProfileView(self, self.core),
            'choose-profile', 'profiles view')
        self.stack.add_titled(NewProfileView(self, self.core),
            'new-profile', 'new profile view')
        self.stack.add_titled(MediasView(self, self.core), 'medias',
            'medias view')
        self.stack.add_titled(SetProfilePictureView(self, self.core),
            'set-profile-picture', 'set the profile picture')
        self.add(self.stack)
        #self.fullscreen()
        self.show_all()

    def change_view(self, view):
        '''Change the current view.'''
        self.last_view = self.stack.get_visible_child()
        self.last_view.visible = False
        if isinstance(view, str):
            self.stack.set_visible_child_name(view)
        else:
            self.stack.set_visible_child(view)
        # Notify the view that it is the new visible view
        new_child = self.stack.get_visible_child()
        new_child.visible = True
        new_child.shown()

    def back(self):
        '''Go to the last view.'''
        self.change_view(self.last_view)

    def _delete_clicked(self, widget, events):
        '''X button clicked to close the window.'''
        return self.stack.get_visible_child().exit_clicked()

    def _key_pressed(self, widget, event):
        '''A key has been pressed.'''
        if event.keyval == KEY_FULLSCREEN:
            if self.is_fullscreen:
                self.unfullscreen()
            else:
                self.fullscreen()

    def _state_changed(self, widget, event):
        '''Update the fullscreen state.'''
        self.is_fullscreen = bool(
            event.new_window_state & Gdk.WindowState.FULLSCREEN)


class MenuBar(Gtk.Box):
    '''Base class for the menu bar.'''

    def __init__(self, listener):
        Gtk.Box.__init__(self, orientation=Gtk.Orientation.HORIZONTAL)
        self.listener = listener
        self.__build_widget()
        self.__set_styles()

    def __build_widget(self):
        '''Build the elements of this widget.'''
        # Add the TVFamily logo
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            LOGO_PATH, LOGO_WIDTH, -1, True)
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


class ImageButton(Gtk.Button):
    '''A button with an image.'''

    def __init__(self, images, size, callback, style=None):
        Gtk.Button.__init__(self)
        self.__build_widget(images, size, callback)
        self.__set_styles(style)

    def __build_widget(self, images, size, callback):
        '''Build the elements of this widget.'''
        self.pixbuf_black = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            images[0], *size, True)
        self.pixbuf_white = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            images[1], *size, True)
        self.icon = Gtk.Image.new_from_pixbuf(self.pixbuf_black)
        self.add(self.icon)
        self.connect('focus-in-event', self._focus_in)
        self.connect('focus-out-event', self._focus_out)
        self.connect('enter-notify-event', self._focus_in)
        self.connect('leave-notify-event', self._focus_out)
        self.connect('clicked', callback)

    def __set_styles(self, style):
        '''Configure styles for this widget.'''
        if style is not None:
            self.get_style_context().add_class(style)

    def _focus_in(self, widget, event):
        '''The button is focused.'''
        self.icon.set_from_pixbuf(self.pixbuf_white)

    def _focus_out(self, widget, event):
        '''The button is not focused any more.'''
        self.icon.set_from_pixbuf(self.pixbuf_black)


class MenuBarButton(Gtk.Button):
    '''A button in the menu bar.'''

    def __init__(self, label, callback, data=None):
        Gtk.Button.__init__(self, label)
        self.set_size_request(width=120, height=-1)
        self.get_style_context().add_class('bar-button')
        self.connect('clicked', callback, data)


class ViewButton(Gtk.Button):
    '''A button in any view.'''

    def __init__(self, label='', callback=None, data=None):
        Gtk.Button.__init__(self, label)
        self.get_style_context().add_class('view-button')
        if callback:
            self.connect('clicked', callback, data)


class ViewLabel(Gtk.Label):
    '''A label with shadow.'''

    def __init__(self, text):
        Gtk.Label.__init__(self, text)
        self.get_style_context().add_class('view-label')


class Message(Gtk.MessageDialog):
    '''A dialog with a personalized style.'''

    # Message types
    QUESTION = 0
    ERROR = 1

    _PARAMS = [
        (Gtk.MessageType.QUESTION, Gtk.ButtonsType.YES_NO),
        (Gtk.MessageType.ERROR, Gtk.ButtonsType.OK),
    ]

    def __init__(self, window, type_, message):
        Gtk.MessageDialog.__init__(self, window, 0, self._PARAMS[type_][0],
            self._PARAMS[type_][1], message)
        self.get_style_context().add_class('message')
        self.set_border_width(30)
        self.get_action_area().set_spacing(40)
        self.set_decorated(False)


class ProfileButton(Gtk.Button):
    '''A button to choose a profile.'''

    def __init__(self, name, callback):
        Gtk.Button.__init__(self)
        self.name = name
        self.__build_widget(callback)
        self.__set_styles()

    def __build_widget(self, callback):
        '''Build the elements of this widget.'''
        # Create a box inside the button that will contain the picture and the
        # label
        self.box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.add(self.box)
        # Add the image
        self.picture = Gtk.Image()
        self.picture.set_size_request(*PROFILE_PICTURE_SIZE)
        self.box.pack_start(self.picture, True, True, 0)
        # Add the profile label
        self.box.pack_start(Gtk.Label(self.name), False, False, 0)
        # Connect the widget to the callback
        self.connect('clicked', callback, self.name)

    def __set_styles(self):
        '''Configure the styles for this button.'''
        self.get_style_context().add_class('profile-button')

    def set_picture(self, pixbuf):
        '''Set the given picture to this profile entry.'''
        self.picture.set_from_pixbuf(pixbuf)


class CropImage(Gtk.Box):
    '''A widget to show an image and crop it.'''

    MASK_COLOR = (1.0, 0.5, 0.31)
    ZOOM_STEP = 24
    INITIAL_WINDOW_SIZE = 256

    # Zoom icon constants
    ZOOM_ICON_BLACK = os.path.join(STATIC_PATH, 'zoom-black.svg')
    ZOOM_ICON_WHITE = os.path.join(STATIC_PATH, 'zoom-white.svg')
    ZOOM_IN_ICON_SIZE = (32, 32)
    ZOOM_OUT_ICON_SIZE = (24, 24)

    def __init__(self, size):
        Gtk.Box.__init__(self, orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.size = size
        self.pixbuf = None
        self.mask = cairo.ImageSurface(cairo.FORMAT_A1, *self.size)
        self.__build_widget()
        self.__set_styles()

    def __build_widget(self):
        '''Build the elements of this widget.'''
        # Create the drawing area
        self.vbox_darea = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        self.pack_start(self.vbox_darea, False, False, 0)
        self.drawing_area = Gtk.DrawingArea()
        self.drawing_area.set_size_request(*self.size)
        self.drawing_area.connect('draw', self._draw)
        self.vbox_darea.pack_start(self.drawing_area, True, False, 0)
        # Create the controlling buttons
        hbox_buttons = Gtk.Box(
            orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        self.pack_start(hbox_buttons, False, False, 0)
        self.left_button = ViewButton('◀', self._move, (0, -1))
        hbox_buttons.pack_start(self.left_button, True, True, 0)
        self.right_button = ViewButton('▶', self._move, (0, 1))
        hbox_buttons.pack_start(self.right_button, True, True, 0)
        self.up_button = ViewButton('▲', self._move, (1, -1))
        hbox_buttons.pack_start(self.up_button, True, True, 0)
        self.down_button = ViewButton('▼', self._move, (1, 1))
        hbox_buttons.pack_start(self.down_button, True, True, 0)
        zoom_icons = [self.ZOOM_ICON_BLACK, self.ZOOM_ICON_WHITE]
        self.zoom_in_button = ImageButton(zoom_icons, self.ZOOM_IN_ICON_SIZE,
            self._zoom_in_clicked, style='view-button')
        hbox_buttons.pack_start(self.zoom_in_button, True, True, 0)
        self.zoom_out_button = ImageButton(zoom_icons, self.ZOOM_OUT_ICON_SIZE,
            self._zoom_out_clicked, style='view-button')
        hbox_buttons.pack_start(self.zoom_out_button, True, True, 0)

    def __set_styles(self):
        '''Configure styles for this widget.'''
        self.get_style_context().add_class('crop-image')
        self.vbox_darea.set_name('profile-picture-editor')

    def _move(self, widget, data):
        '''Move the window over the image.'''
        dim, direction = data
        self.window_pos[dim] += (self.step[0] * direction)
        self._adjust_window_pos()
        self.drawing_area.queue_draw()

    def _zoom_in_clicked(self, widget):
        '''Button 'zoom in' of this widget has been clicked.'''
        self.window_size += self.ZOOM_STEP
        self._adjust_window_size()
        self._adjust_window_pos()
        self.drawing_area.queue_draw()

    def _zoom_out_clicked(self, widget):
        '''Button 'zoom out' of this widget has been clicked.'''
        self.window_size -= self.ZOOM_STEP
        self._adjust_window_size()
        self.drawing_area.queue_draw()

    def _adjust_window_pos(self):
        '''Make sure the whole window is over the image.'''
        wp = self.window_pos
        for i in range(2):
            if wp[i] < 0:
                wp[i] = 0
            elif wp[i] + self.window_size > self.original_size[i]:
                wp[i] = self.original_size[i] - self.window_size

    def _adjust_window_size(self):
        '''Make sure the window is not bigger than the image.'''
        for i in range(2):
            if self.window_size > self.original_size[i]:
                self.window_size = self.original_size[i]
        if self.window_size < 1:
            self.window_size = 1

    def _draw(self, widget, context):
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
            self._adjust_window_size()
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


class BusyDialog(Gtk.Dialog):
    '''A modal dialog to wait for some process.'''

    RESPONSE_OK = 0
    RESPONSE_ERROR = 1

    def __init__(self, label, parent):
        Gtk.Dialog.__init__(self, label, parent, Gtk.DialogFlags.MODAL)
        self.__build_widget(label)
        self.__set_styles()

    def __build_widget(self, label):
        '''Build this widget.'''
        box = self.get_content_area()
        box.pack_start(Gtk.Label(label), False, False, 0)
        spinner = Gtk.Spinner()
        spinner.set_size_request(100, 100)
        box.pack_start(spinner, False, False, 0)
        spinner.start()
        self.set_size_request(400, -1)
        self.set_border_width(30)
        self.set_decorated(False)
        self.show_all()

    def __set_styles(self):
        '''Configure styles for this widget.'''
        self.get_style_context().add_class('message')

    def set_response(self, response_ok):
        '''Force this dialog to generate a response.'''
        if response_ok:
            response = BusyDialog.RESPONSE_OK
        else:
            response = BusyDialog.RESPONSE_ERROR
        GLib.idle_add(self.response, response)


class ProfileMenu(Gtk.Button):
    '''The profile menu.'''

    def __init__(self, listener):
        Gtk.Button.__init__(self)
        self.__build_widget(listener)
        self.__set_styles()

    def __build_widget(self, listener):
        '''Build this widget.'''
        # Build the button with a label and an image
        hbox_button = Gtk.Box(
            orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        self.add(hbox_button)
        self.label = Gtk.Label()
        hbox_button.pack_start(self.label, False, False, 0)
        # Add the image
        self.picture = Gtk.Image()
        self.picture.set_size_request(*PROFILE_PICTURE_SIZE_SMALL)
        hbox_button.pack_start(self.picture, True, True, 0)
        # Create the popover menu
        self.popover = Gtk.Popover()
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.popover.add(box)
        # Create the buttons that go into the popover
        self.change_picture = self._button(
            'Change picture', box, listener.change_profile_picture)
        self.settings = self._button('Settings', box, listener.settings)
        self.change_profile = self._button(
            'Change profile', box, listener.change_profile)
        self.delete_profile = self._button(
            'Delete profile', box, listener.delete_profile)
        self.quit = self._button('Quit', box, listener.exit_clicked)
        # Connect actions
        self.connect('clicked', self._clicked)

    def _button(self, label, box, callback):
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

    def _clicked(self, widget):
        '''The menu button has been clicked. Show the menu.'''
        self.popover.set_relative_to(self)
        self.popover.show_all()
        self.popover.popup()


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
            "Are you sure you want to exit?")
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
        self.bar = MenuBar(self)
        self.pack_start(self.bar, False, False, 0)
        self.contents_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.pack_start(self.contents_box, True, True, 0)


class SplashView(View):
    '''The splash view.'''

    def __init__(self, window):
        View.__init__(self, window)
        self.__build_widget()
        GLib.timeout_add_seconds(
            SPLASH_SECONDS, self.window.change_view, 'choose-profile')

    def __build_widget(self):
        '''Build the elements of this widget.'''
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            LOGO_PATH, SPLASH_ICON_WIDTH, -1, True)
        logo = Gtk.Image.new_from_pixbuf(pixbuf)
        self.pack_start(logo, True, False, 0)


class ChooseProfileView(MenuBarView):
    '''The view to choose the profile.'''

    def __init__(self, window, core):
        MenuBarView.__init__(self, window, core)
        self.__build_widget()
        self.__set_styles()

    def __build_widget(self):
        '''Build the elements of this widget.'''
        # Configure the menu bar and the contents area
        self.bar.add(back=[new_exit_button(self)])
        self.contents_box.set_spacing(20)
        self.contents_box.props.valign = Gtk.Align.CENTER
        # Create the horizontal box that will contain the profiles buttons
        self.profiles_box = Gtk.Box(
            orientation=Gtk.Orientation.HORIZONTAL, spacing=20)
        self.profiles_box.props.halign = Gtk.Align.CENTER
        self.contents_box.pack_start(self.profiles_box, False, False, 0)
        # Create an horizontal box that contain the 'New profile button',
        # because if it is put in the vertical box it expandes horizontally,
        # and we don't want that
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        self.contents_box.pack_start(hbox, False, False, 0)
        # Create the 'New profile button'
        self.new_profile_button = ViewButton('New profile', self.new_profile)
        hbox.pack_start(self.new_profile_button, True, False, 0)

    def __set_styles(self):
        '''Configure the styles.'''
        self.profiles_box.set_name('profiles-box')

    def shown(self):
        '''Executed when this view is visible.'''
        self.error_get_profiles = False
        self._empty_profiles_box()
        self.idle_id = GLib.idle_add(
            self.core.request_profiles_list, self.set_profiles_list)

    def _empty_profiles_box(self):
        '''Remove all the children in the profiles box.'''
        for c in self.profiles_box.get_children():
            self.profiles_box.remove(c)

    def set_profiles_list(self, profiles=None, error=None):
        '''Callback executed by the core to set the list of profiles.'''
        GLib.idle_add(self._update_profiles, profiles, error)

    def _update_profiles(self, profiles, error):
        '''Update the list of profiles in the screen.'''
        if self.visible:
            if error:
                # Some error ocurred
                if not self.error_get_profiles:
                    message(self.window, Message.ERROR, str(error))
                    # Add a message in the profiles box and retry after some
                    # time
                    label = ViewLabel(str(error))
                    self.profiles_box.pack_start(label, False, False, 0)
                    self.error_get_profiles = True
                self.idle_id = GLib.timeout_add_seconds(
                    GET_PROFILES_RETRY_SECONDS,
                    self.core.request_profiles_list, self.set_profiles_list)
            else:
                self.idle_id = None
                self._empty_profiles_box()
                if not profiles:
                    label = ViewLabel('No profiles available')
                    self.profiles_box.pack_start(label, False, False, 0)
                else:
                    # Add an element for each profile
                    for p in profiles:
                        b = ProfileButton(p, self.profile_clicked)
                        self.profiles_box.pack_start(b, False, False, 0)
                    # Request the profiles pictures
                    for p in profiles:
                        self.core.request_profile_picture(
                            p, self.set_profile_picture)
            self.show_all()
            self._set_default_focus()

    def _set_default_focus(self):
        '''Set the default focus.'''
        child = self.profiles_box.get_children()[0]
        if isinstance(child, Gtk.Button):
            child.grab_focus()
        else:
            self.new_profile_button.grab_focus()

    def set_profile_picture(self, name, picture=None, error=None):
        '''Set the picture for a given profile.'''
        save_picture_to_file(picture, os.path.join(TMP_DIR, name + '.png'))
        # Load the picture in the main loop
        GLib.idle_add(self._update_profile_picture, name)

    def _update_profile_picture(self, name):
        '''Update the picture of a given profile.'''
        if self.visible:
            picture_file = os.path.join(TMP_DIR, name + '.png')
            pixbuf = load_picture_from_file(picture_file, PROFILE_PICTURE_SIZE)
            # Update the profile image
            for profile in self.profiles_box.get_children():
                if profile.name == name:
                    profile.set_picture(pixbuf)

    def new_profile(self, widget, data=None):
        '''Go to the screen 'New profile'.'''
        if self.idle_id is not None:
            GLib.source_remove(self.idle_id)
        self.window.change_view('new-profile')

    def profile_clicked(self, widget, name):
        '''A profile has been chosen.'''
        self.core.set_profile(name)
        self.window.change_view('medias')


class EditProfileView(MenuBarView):
    '''Base class for the view to create a new profile or edit its picture.'''

    CROP_IMAGE_SIZE = (320, 240)
    PICTURE_TYPE = 'png'

    def __init__(self, window, core):
        MenuBarView.__init__(self, window, core)
        self.__build_widget()
        self.__set_styles()

    def __build_widget(self):
        '''Build the elements of this widget.'''
        # Configure the menu bar
        self.bar.add(back=[MenuBarButton('Back', self.leave)])
        # Configure the contents box
        self.contents_box.props.valign = Gtk.Align.CENTER
        hbox_main = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        hbox_main.props.halign = Gtk.Align.CENTER
        self.contents_box.pack_start(hbox_main, False, False, 0)
        self.vbox_main = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.vbox_main.set_spacing(30)
        hbox_main.pack_start(self.vbox_main, False, False, 0)
        # Create a button to choose the profile picture
        vbox_file = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.vbox_main.pack_start(vbox_file, False, False, 0)
        hbox_label_file = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        vbox_file.pack_start(hbox_label_file, False, False, 0)
        self.picture_label = ViewLabel('')
        hbox_label_file.pack_start(self.picture_label, False, False, 0)
        self.filebutton = ViewButton('None', self.choose_picture)
        self.filebutton.set_size_request(600, -1)
        vbox_file.pack_start(self.filebutton, False, False, 0)
        # Create the control to crop the selected picture
        self.crop_image = CropImage(self.CROP_IMAGE_SIZE)
        self.vbox_main.pack_start(self.crop_image, False, False, 0)
        # Create the 'Create' button
        hbox_create = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        self.vbox_main.pack_start(hbox_create, False, False, 0)
        self.action_button = ViewButton(callback=self.action)
        hbox_create.pack_start(self.action_button, True, False, 0)

    def __set_styles(self):
        '''Configure the styles.'''
        self.contents_box.set_name('new-profile-contents-box')

    def choose_picture(self, widget, data=None):
        '''Set the picture to edit.'''
        dialog = Gtk.FileChooserDialog("Choose the profile picture",
            self.window, Gtk.FileChooserAction.OPEN,
            (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
            Gtk.STOCK_OPEN, Gtk.ResponseType.OK))
        filter = Gtk.FileFilter()
        filter.set_name("JPEG Images")
        filter.add_mime_type("image/jpeg")
        dialog.add_filter(filter)
        filter = Gtk.FileFilter()
        filter.set_name("PNG Images")
        filter.add_mime_type("image/png")
        dialog.add_filter(filter)
        response = dialog.run()
        filename = dialog.get_filename()
        dialog.destroy()
        if response == Gtk.ResponseType.OK:
            try:
                self.crop_image.set_image(filename)
                self.filebutton.set_label(os.path.basename(filename))
            except GLib.Error:
                message(self.window, Message.ERROR,
                    'Picture format not supported')

    def save_picture_to_file(self, name):
        tempimage = None
        if self.filebutton.get_label() != 'None':
            # Get the cropped image and save it into a temporary file
            pixbuf = self.crop_image.get_cropped_image()
            tempimage = os.path.join(TMP_DIR, '{}.png'.format(name))
            pixbuf.savev(tempimage, self.PICTURE_TYPE, [None], [None])
        return tempimage

    def busy_wait(self, image, msg):
        '''Show a busy dialog and when finished, perform the cleanup.'''
        response = self.busy_dialog.run()
        self.busy_dialog.destroy()
        # Check if the profile was succesfully created
        if response == BusyDialog.RESPONSE_OK:
            self.leave(self)
        else:
            message(self.window, Message.ERROR, '{}: {}'.format(
                msg, self.error))
        if image is not None:
            os.unlink(image)

    def done(self, name, error=None):
        '''Callback executed when the profile has been created.'''
        self.error = error
        self.busy_dialog.set_response(error is None)

    def leave(self, widget=None):
        '''Leave this view.'''
        self.window.back()
        self.filebutton.set_label('None')
        self.crop_image.set_image(None)


class NewProfileView(EditProfileView):
    '''The view to create a profile.'''

    def __init__(self, window, core):
        EditProfileView.__init__(self, window, core)
        self.__build_widget()

    def __build_widget(self):
        '''Build the elements of this widget.'''
        # Create the controls for the profile's name
        vbox_name = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.vbox_main.pack_start(vbox_name, False, False, 0)
        self.vbox_main.reorder_child(vbox_name, 0)
        hbox_label_name = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        vbox_name.pack_start(hbox_label_name, False, False, 0)
        label = ViewLabel("New profile's name")
        hbox_label_name.pack_start(label, False, False, 0)
        self.entry = Gtk.Entry()
        vbox_name.pack_start(self.entry, False, False, 0)
        # Configure the picture's label
        self.picture_label.set_text('Profile picture')
        # Configure the action button's label
        self.action_button.set_label('Create')

    def shown(self):
        '''This view is shown.'''
        self.entry.grab_focus()

    def action(self, widget, data=None):
        '''Create a profile.'''
        name = self.entry.get_text()
        if not name:
            message(self.window, Message.ERROR,
                'The profile name cannot be empty')
        else:
            tempimage = self.save_picture_to_file(name)
            # Create the profile
            self.busy_dialog = BusyDialog('Creating profile...', self.window)
            self.core.request_create_profile(name, tempimage, self.done)
            # Wait for the operation to complete and cleanup
            self.busy_wait(tempimage, 'Cannot create the profile')

    def leave(self, widget=None):
        '''Leave this view.'''
        EditProfileView.leave(self, widget)
        self.entry.set_text('')


class SetProfilePictureView(EditProfileView):
    '''The view to set a new profile picture.'''

    def __init__(self, window, core):
        EditProfileView.__init__(self, window, core)
        self.__build_widget()

    def __build_widget(self):
        '''Build the elements of this widget.'''
        # Configure the action button's label
        self.action_button.set_label('Save')

    def shown(self):
        '''This view is shown.'''
        # Configure the picture's label
        self.picture_label.set_text(
            'New picture for {}'.format(self.core.get_profile()))
        self.filebutton.grab_focus()

    def action(self, widget, data=None):
        '''Update the profile image.'''
        tempimage = self.save_picture_to_file(self.core.get_profile())
        # Update the profile picture
        self.busy_dialog = BusyDialog('Updating profile...', self.window)
        self.core.request_set_profile_picture(tempimage, self.done)
        # Wait for the operation to complete and cleanup
        self.busy_wait(tempimage, 'Cannot update the profile')


class MediasView(MenuBarView):
    '''The view to choose a media.'''

    def __init__(self, window, core):
        MenuBarView.__init__(self, window, core)
        self.__build_widget()

    def __build_widget(self):
        '''Build the elements of this widget.'''
        # Configure the menu bar and the contents area
        self.profile_menu = ProfileMenu(self)
        self.bar.add(back=[self.profile_menu])
        self.core.request_categories(self.set_categories)

    def shown(self):
        '''This view is shown.'''
        profile = self.core.get_profile()
        self.profile_menu.set_name(profile)
        self.core.request_profile_picture(profile, self.set_profile_picture)

    def set_profile_picture(self, name, picture=None, error=None):
        '''Set the picture for a given profile.'''
        self.picture_file = os.path.join(TMP_DIR, name + '.png')
        save_picture_to_file(picture, self.picture_file)
        # Load the picture in the main loop
        GLib.idle_add(self._update_profile_picture, name)

    def _update_profile_picture(self, name):
        '''Update the picture of a given profile.'''
        if self.visible:
            pixbuf = load_picture_from_file(
                self.picture_file, PROFILE_PICTURE_SIZE_SMALL)
            # Update the profile image
            self.profile_menu.set_picture(pixbuf)

    def set_categories(self, categories=None, error=None):
        '''Set the list of categories.'''
        if not error:
            GLib.idle_add(self._update_categories, categories)

    def _update_categories(self, categories):
        '''Update the list of categories in the menu.'''
        l = [MenuBarButton(c, self.category_clicked, c) for c in categories]
        self.bar.add(front=l)

    def change_profile_picture(self, widget):
        self.window.change_view('set-profile-picture')

    def settings(self, widget):
        print('settings')

    def change_profile(self, widget=None):
        self.core.set_profile(None)
        self.window.change_view('choose-profile')

    def delete_profile(self, widget):
        profile = self.core.get_profile()
        response = message(self.window, Message.QUESTION,
            "Delete the profile {}?".format(profile))
        if response == Gtk.ResponseType.YES:
            self.busy_dialog = BusyDialog('Deleting profile...', self.window)
            self.core.request_delete_profile(self.profile_deleted)
            # Show a busy dialog
            response = self.busy_dialog.run()
            self.busy_dialog.destroy()
            # Check if the profile was succesfully deleted
            if response == BusyDialog.RESPONSE_OK:
                self.change_profile()
            else:
                message(self.window, Message.ERROR,
                    'Cannot delete the profile: {}'.format(self.error))

    def profile_deleted(self, name, error=None):
        '''Callback executed when the profile has been deleted.'''
        self.error = error
        self.busy_dialog.set_response(error is None)

    def category_clicked(self, widget, category):
        print('category {}'.format(category))

