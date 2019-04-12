
'''gui.py - Windows layer.

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

# TODO: signals handling
# TODO: Change the characters of the crop image buttons by images

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GLib, Gdk, GdkPixbuf
import math
import os
import tempfile
import time

import tvfamilygtk.core
from tvfamilygtk import widgets as tfw
from tvfamilygtk import paths as tfp

__author__ = 'Antonio Serrano Hernandez'
__copyright__ = 'Copyright (C) 2018 2019 Antonio Serrano Hernandez'
__version__ = '0.1'
__license__ = 'GPL'
__maintainer__ = 'Antonio Serrano Hernandez'
__email__ = 'toni.serranoh@gmail.com'
__status__ = 'Development'
__homepage__ = 'https://github.com/aserranoh/tvfamily-gtk'


TIMEOUT_REQUEST = 5


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
        with open(tfp.get_styles(), 'rb') as f:
            css_provider.load_from_data(f.read())
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(),
            css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    def run(self):
        Gtk.main()


class MainWindow(Gtk.Window):
    '''The application window.'''

    KEY_FULLSCREEN = Gdk.KEY_F11

    def __init__(self, core):
        Gtk.Window.__init__(self, title='TVFamily GTK')
        self.core = core
        # Build the widget
        self.__build()

    def __build(self):
        '''Build this widget.'''
        # Add the quit signal handler
        self.connect('delete-event', self.delete_clicked)
        self.set_size_request(1280, 720)
        #self.set_size_request(1920, 1080)
        # Connect the signals to switch to fullscreen
        self.is_fullscreen = False
        self.connect('key-press-event', self.key_pressed)
        self.connect('window-state-event', self.state_changed)
        # Build the stack that contains all the views
        self.stack = Gtk.Stack()
        self.add(self.stack)
        self.show_all()
        self.stack.add_named(SplashView(self), 'splash')
        self.stack.add_named(ChooseProfileView(self, self.core),
            'choose-profile')
        self.stack.add_named(NewProfileView(self, self.core), 'new-profile')
        self.stack.add_named(MediasView(self, self.core), 'medias')
        self.stack.add_named(SetProfilePictureView(self, self.core),
            'set-profile-picture')
        self.stack.add_named(VideoView(self, self.core), 'video')
        #self.fullscreen()

    def change_view(self, view, **kwargs):
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
        new_child.shown(**kwargs)

    def back(self):
        '''Go to the last view.'''
        self.change_view(self.last_view)

    def delete_clicked(self, widget, events):
        '''X button clicked to close the window.'''
        return self.stack.get_visible_child().exit_clicked()

    def key_pressed(self, widget, event):
        '''A key has been pressed.'''
        if event.keyval == self.KEY_FULLSCREEN:
            if self.is_fullscreen:
                self.unfullscreen()
            else:
                self.fullscreen()
        self.stack.get_visible_child().key_pressed(widget, event)

    def state_changed(self, widget, event):
        '''Update the fullscreen state.'''
        self.is_fullscreen = bool(
            event.new_window_state & Gdk.WindowState.FULLSCREEN)


class SplashView(tfw.View):
    '''The splash view.'''

    SPLASH_ICON_WIDTH = 800
    SPLASH_SECONDS = 0

    def __init__(self, window):
        tfw.View.__init__(self, window)
        self.__build()
        GLib.timeout_add_seconds(
            self.SPLASH_SECONDS, self.window.change_view, 'choose-profile')

    def __build(self):
        '''Build the elements of this widget.'''
        pixbuf = GdkPixbuf.Pixbuf.new_from_file_at_scale(
            tfp.get_logo(), self.SPLASH_ICON_WIDTH, -1, True)
        logo = Gtk.Image.new_from_pixbuf(pixbuf)
        self.pack_start(logo, True, False, 0)
        self.show_all()


class ChooseProfileView(tfw.MenuBarView):
    '''The view to choose the profile.'''

    PROFILE_PICTURE_SIZE = (128, 128)

    def __init__(self, window, core):
        tfw.MenuBarView.__init__(self, window, core)
        self.__build()
        self.requests = tfw.ServerRequestList()
        self.profile_get_focus = False
        self.button_get_focus = False

    def __build(self):
        '''Build the elements of this widget.'''
        # Configure the menu bar and the contents area
        exit_button = tfw.ImageButton(
            [tfp.get_image('off-black'), tfp.get_image('off-white')],
            (-1, self.bar.height), self.exit_clicked, style='bar-button')
        self.bar.add(back=[exit_button])
        self.contents_box.set_spacing(20)
        self.contents_box.props.valign = Gtk.Align.CENTER
        # Create the container for the profiles buttons
        self.profiles_box = tfw.ProfileButtonsBox(
            self.PROFILE_PICTURE_SIZE, self.profile_clicked)
        # Create the label Connection error
        self.label = tfw.Label(styles=['view-label'])
        # Add a stack to alternate between the label and the list of profiles
        self.stack = Gtk.Stack()
        self.contents_box.pack_start(self.stack, True, True, 0)
        self.stack.add_named(self.profiles_box, 'profiles')
        self.stack.add_named(self.label, 'label')
        # Create an horizontal box that contain the 'New profile button',
        # because if it is put in the vertical box it expandes horizontally,
        # and we don't want that
        # TODO: Cannot do that without an hbox?
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        self.contents_box.pack_start(hbox, False, False, 0)
        # Create the 'New profile button'
        self.new_profile_button = tfw.ViewButton(
            'New profile', self.new_profile)
        hbox.pack_start(self.new_profile_button, True, False, 0)
        self.show_all()
        self.contents_box.hide()

    def shown(self):
        '''Executed when this view is visible.'''
        self.requests.add(self.core.get_profiles, (), self.update_profiles,
            TIMEOUT_REQUEST)

    def update_profiles(self, request):
        '''Update the list of profiles in the screen.'''
        if request.error:
            # Some error ocurred, add a message in the profiles box
            self.label.set_text(str(request.error))
            self.stack.set_visible_child(self.label)
        else:
            if not request.result:
                self.label.set_text('No profiles available')
                self.stack.set_visible_child(self.label)
            else:
                # Set the profiles list
                self.profiles_box.set_profiles(request.result)
                # Request the profiles pictures
                for p in request.result:
                    self.requests.add(self.core.get_profile_picture, (p,), 
                        self.update_profile_picture, TIMEOUT_REQUEST)
                self.stack.set_visible_child(self.profiles_box)
        self.contents_box.show_all()
        self.set_default_focus()

    def set_default_focus(self):
        '''Set the default focus.'''
        if not self.profile_get_focus:
            try:
                self.profiles_box.set_profile_focused(0)
                self.profile_get_focus = True
            except IndexError:
                if not self.button_get_focus:
                    self.new_profile_button.grab_focus()
                    self.button_get_focus = True

    def update_profile_picture(self, request):
        '''Update the picture of a given profile.'''
        if request.result:
            img = tfw.Image(request.result, self.PROFILE_PICTURE_SIZE)
        else:
            img = tfw.Image(
                tfp.get_default_picture(), self.PROFILE_PICTURE_SIZE)
        self.profiles_box.set_profile_picture(request.args[0], img.pixbuf)

    def new_profile(self, widget, data=None):
        '''Go to the screen 'New profile'.'''
        self.leave('new-profile')

    def profile_clicked(self, widget):
        '''A profile has been chosen.'''
        self.core.set_profile(widget.get_name())
        self.leave('medias')

    def leave(self, new_view):
        '''Leave this view.'''
        self.requests.cancel_all()
        self.profile_get_focus = False
        self.button_get_focus = False
        self.contents_box.hide()
        self.window.change_view(new_view)


class EditProfileView(tfw.MenuBarView):
    '''Base class for the view to create a new profile or edit its picture.'''

    CROP_IMAGE_SIZE = (320, 240)
    PICTURE_TYPE = 'png'

    def __init__(self, window, core):
        tfw.MenuBarView.__init__(self, window, core)
        self.__build()
        self.__set_styles()

    def __build(self):
        '''Build the elements of this widget.'''
        # Configure the menu bar
        self.bar.add(back=[tfw.MenuBarButton('Back', self.leave)])
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
        self.picture_label = tfw.Label(styles=['view-label'])
        hbox_label_file.pack_start(self.picture_label, False, False, 0)
        self.filebutton = tfw.ViewButton('None', self.choose_picture)
        self.filebutton.set_size_request(600, -1)
        vbox_file.pack_start(self.filebutton, False, False, 0)
        # Create the control to crop the selected picture
        self.crop_image = tfw.CropImage(self.CROP_IMAGE_SIZE)
        self.vbox_main.pack_start(self.crop_image, False, False, 0)
        # Create the 'Create' button
        hbox_create = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        self.vbox_main.pack_start(hbox_create, False, False, 0)
        self.action_button = tfw.ViewButton(callback=self.action)
        hbox_create.pack_start(self.action_button, True, False, 0)

    def __set_styles(self):
        '''Configure the styles.'''
        self.contents_box.set_name('new-profile-contents-box')

    def __enter__(self):
        '''Simply return the temporary filename.'''
        return self.filename

    def __exit__(self, *args):
        '''Remove the temporary picture.'''
        if self.filename:
            os.unlink(self.filename)

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

    def get_temp_picture(self):
        '''Save the temporary picture into a file and return self as a context
        manager.
        '''
        self.filename = None
        if self.filebutton.get_label() != 'None':
            fd, self.filename = tempfile.mkstemp()
            os.close(fd)
            pixbuf = self.crop_image.get_cropped_image()
            pixbuf.savev(self.filename, self.PICTURE_TYPE, [None], [None])
        return self

    def leave(self, widget=None, data=None):
        '''Leave this view.'''
        self.window.back()
        self.filebutton.set_label('None')
        self.crop_image.set_image(None)


class NewProfileView(EditProfileView):
    '''The view to create a profile.'''

    def __init__(self, window, core):
        EditProfileView.__init__(self, window, core)
        self.__build()

    def __build(self):
        '''Build the elements of this widget.'''
        # Create the controls for the profile's name
        vbox_name = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)
        self.vbox_main.pack_start(vbox_name, False, False, 0)
        self.vbox_main.reorder_child(vbox_name, 0)
        hbox_label_name = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        vbox_name.pack_start(hbox_label_name, False, False, 0)
        label = tfw.Label("New profile's name", styles=['view-label'])
        hbox_label_name.pack_start(label, False, False, 0)
        self.entry = Gtk.Entry()
        vbox_name.pack_start(self.entry, False, False, 0)
        # Configure the picture's label
        self.picture_label.set_text('Profile picture')
        # Configure the action button's label
        self.action_button.set_label('Create')
        self.show_all()

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
            with self.get_temp_picture() as pic:
                # Create the profile
                try:
                    self.core.create_profile(name, pic)
                    self.leave()
                except Exception as e:
                    tfw.message(self.window, tfw.Message.ERROR,
                        '{}: {}'.format('Cannot create the profile', e))

    def leave(self, widget=None, data=None):
        '''Leave this view.'''
        EditProfileView.leave(self)
        self.entry.set_text('')


class SetProfilePictureView(EditProfileView):
    '''The view to set a new profile picture.'''

    def __init__(self, window, core):
        EditProfileView.__init__(self, window, core)
        self.__build()

    def __build(self):
        '''Build the elements of this widget.'''
        # Configure the action button's label
        self.action_button.set_label('Save')
        self.show_all()

    def shown(self):
        '''This view is shown.'''
        # Configure the picture's label
        self.picture_label.set_text(
            'New picture for {}'.format(self.core.get_profile()))
        self.filebutton.grab_focus()

    def action(self, widget, data=None):
        '''Perform the action of this view.'''
        # Get the pixbuf and save it into a file
        with self.get_temp_picture() as f:
            try:
                # Update the profile picture
                self.core.set_profile_picture(f)
                self.leave()
            except Exception as e:
                tfw.message(self.window, tfw.Message.ERROR, '{}: {}'.format(
                    'Cannot update the profile', e))


class MediasView(tfw.MenuBarView):
    '''The view to choose a media.'''

    MEDIAS_BOX_NUM_COLS = 4
    POSTER_BORDER = 4
    POSTER_RATIO = 268/182

    def __init__(self, window, core):
        tfw.MenuBarView.__init__(self, window, core)
        self.__build()
        self.category_buttons = None
        self.current_category = None
        self.requests = tfw.ServerRequestList()
        self.current_request = None
        self.pixbuf_cache = tfw.PixbufCache()
        # Request the categories when this view is created
        self.requests.add(self.core.get_categories, (),
            self.update_categories, TIMEOUT_REQUEST)

    def __build(self):
        '''Build the elements of this widget.'''
        # Configure the menu bar and the contents area
        self.profile_menu = tfw.ProfileMenu(
            self.get_profile_picture_size(), self)
        self.bar.add(back=[self.profile_menu])
        # Add a stack to alternate between a message and the list of medias
        self.stack = Gtk.Stack()
        self.contents_box.pack_start(self.stack, True, True, 0)
        # The label to display a message
        self.label = tfw.Label('No available medias', styles=['view-label'])
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        self.build_poster_area(hbox)
        self.build_medias_area(hbox)
        self.show_all()
        # Add the different contents to the stack (after showing everything)
        self.stack.add_named(self.label, 'label')
        self.stack.add_named(hbox, 'medias')

    def build_poster_area(self, hbox):
        poster_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=15)
        poster_box.get_style_context().add_class('poster-box')
        hbox.pack_start(poster_box, False, False, 0)
        # Title label
        self.title_label = tfw.Label(styles=['title-label'])
        poster_box.pack_start(self.title_label, False, False, 0)
        # Attrs
        attrs_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=20)
        poster_box.pack_start(attrs_box, False, False, 0)
        self.year_label = tfw.Label(styles=['view-label'])
        attrs_box.pack_start(self.year_label, False, False, 0)
        attrs_box.pack_start(tfw.Label('|', ['sep']), False, False, 0)
        self.genre_label = tfw.Label(styles=['view-label'])
        attrs_box.pack_start(self.genre_label, False, False, 0)
        attrs_box.pack_start(tfw.Label('|', ['sep']), False, False, 0)
        rating_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)
        img = Gtk.Image()
        img.set_from_file(tfp.get_image('star'))
        self.rating_label = tfw.Label(styles=['view-label', 'rating'])
        rating_box.pack_start(img, False, False, 0)
        rating_box.pack_start(self.rating_label, False, False, 0)
        attrs_box.pack_start(rating_box, False, False, 0)
        # Hbox that contains the poster and the plot
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=20)
        poster_box.pack_start(hbox, False, False, 0)
        # Poster image
        self.poster_image = Gtk.Image()
        hbox.pack_start(self.poster_image, False, False, 0)
        # Plot
        # Put it in an Vbox to align it to the top
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        hbox.pack_end(vbox, False, False, 0)
        self.plot_label = tfw.Label(styles=['view-label'])
        self.plot_label.set_line_wrap(True)
        self.plot_label.set_justify(Gtk.Justification.FILL)
        vbox.pack_start(self.plot_label, False, False, 0)

    def build_medias_area(self, hbox):
        self.medias_box = tfw.MediasBox(
            self.MEDIAS_BOX_NUM_COLS, self.media_clicked, self.media_focused)
        hbox.pack_end(self.medias_box, False, False, 0)

    def get_profile_picture_size(self):
        '''Return the profile picture's size to use.'''
        return (self.bar.height,)*2

    def shown(self):
        '''This view is shown.'''
        profile = self.core.get_profile()
        self.profile_menu.set_name(profile)
        self.requests.add(self.core.get_profile_picture, (profile,),
            self.update_profile_picture, TIMEOUT_REQUEST)
        if self.current_category:
            self.category_clicked(self.current_category)
        if len(self.category_buttons):
            self.category_buttons[0].grab_focus()
        # Compute the medias box's poster size
        w = (self.window.get_allocated_width() / 2
            / self.MEDIAS_BOX_NUM_COLS - (2 * self.POSTER_BORDER))
        h = w * self.POSTER_RATIO
        self.medias_box.set_poster_size((w, h))
        # Compute the big poster's size
        w = self.window.get_allocated_width() / 4
        h = w * self.POSTER_RATIO
        self.big_poster_size = (w, h)
        self.poster_image.set_size_request(*self.big_poster_size)

    def update_profile_picture(self, request):
        '''Update the picture of a given profile.'''
        if not request.error and request.result:
            img = tfw.Image(request.result, self.get_profile_picture_size())
        else:
            img = tfw.Image(
                tfp.get_default_picture(), self.get_profile_picture_size())
        self.profile_menu.set_picture(img.pixbuf)

    def update_categories(self, request):
        '''Update the list of categories in the menu.'''
        if not request.error:
            self.category_buttons = [tfw.MenuBarButton(
                c, self.category_clicked) for c in request.result]
            self.bar.add(front=self.category_buttons)
            if request.result:
                self.category_clicked(self.category_buttons[0])

    def change_profile_picture(self, widget):
        self.leave('set-profile-picture')

    def settings(self, widget):
        print('settings')

    def change_profile(self, widget=None):
        self.core.set_profile(None)
        self.leave('choose-profile')

    def delete_profile(self, widget):
        profile = self.core.get_profile()
        response = tfw.message(self.window, tfw.Message.QUESTION,
            "Delete the profile {}?".format(profile))
        if response == Gtk.ResponseType.YES:
            try:
                self.core.delete_profile()
                self.change_profile()
            except Exception as e:
                tfw.message(self.window, tfw.Message.ERROR,
                    'Cannot delete the profile: {}'.format(e))

    def category_clicked(self, widget, data=None):
        '''A category has been clicked.'''
        # Cancel the previous request, if any
        if self.current_request is not None:
            self.requests.cancel(self.current_request)
            self.current_request = None
        # Update the current category button
        if self.current_category:
            self.current_category.get_style_context().remove_class(
                'current-category')
        self.current_category = widget
        widget.get_style_context().add_class('current-category')
        # Retrieve the list of medias
        if self.visible:
            self.current_request = self.requests.add(self.core.get_medias,
                (widget.get_label(),), self.update_medias, TIMEOUT_REQUEST)

    def update_medias(self, request):
        '''Update the list of medias in this view.'''
        # Check that the current category is the one we asked for
        if request.args[0] == self.current_category.get_label():
            if request.error:
                self.label.show()
                self.stack.set_visible_child(self.label)
            else:
                if request.result:
                    self.medias_box.set_medias(request.result)
                    for m in request.result:
                        self.requests.add(self.core.get_poster, (m,),
                            self.update_medias_box_poster, TIMEOUT_REQUEST)
                    self.stack.show_all()
                    self.stack.set_visible_child_name('medias')
                    self.medias_box.select_media(0)
                else:
                    # Show a message that no medias are available
                    self.label.show()
                    self.stack.set_visible_child(self.label)
                self.current_request = None

    def update_medias_box_poster(self, request):
        '''Update a poster.'''
        media = request.args[0]
        if request.error:
            p = self.pixbuf_cache.get_pixbuf(tfp.get_default_picture(),
                self.medias_box.get_poster_size())
        else:
            try:
                p = self.pixbuf_cache.get_pixbuf(request.result,
                    self.medias_box.get_poster_size())
            except GLib.Error as e:
                p = self.pixbuf_cache.get_pixbuf(tfp.get_default_picture(),
                    self.medias_box.get_poster_size())
        self.medias_box.set_poster(media, p)

    def media_clicked(self, widget):
        try:
            widget.media.seasons
            pass
        except AttributeError:
            self.leave('video', media=widget.media)

    def media_focused(self, widget, media):
        '''A media is beeing hovered.'''
        # Ask for the poster
        self.requests.add(self.core.get_poster, (media,),
            self.update_big_poster, TIMEOUT_REQUEST)
        # Set the other attributes
        self.title_label.set_text(str(media))
        self.year_label.set_text(str(media.air_year))
        if isinstance(media.genre, list):
            self.genre_label.set_text(', '.join(media.genre))
        else:
            self.genre_label.set_text(media.genre)
        self.rating_label.set_text(str(media.rating))
        self.plot_label.set_text(media.plot)

    def update_big_poster(self, request):
        '''Update the image on the big poster.'''
        media = request.args[0]
        w = self.big_poster_size[0]
        if request.error:
            i = tfw.Image(tfp.get_default_picture(), (w, -1))
        else:
            try:
                i = tfw.Image(request.result, (w, -1))
            except GLib.Error as e:
                i = tfw.Image(tfp.get_default_picture(), (w, -1))
        self.poster_image.set_from_pixbuf(i.pixbuf)

    def leave(self, new_view, **kwargs):
        '''Leave this view.'''
        self.requests.cancel_all()
        self.stack.hide()
        self.profile_menu.set_picture(None)
        self.window.change_view(new_view, **kwargs)


class VideoView(tfw.View):
    '''Video player view.'''

    HIDE_PLAYER_SECONDS = 3

    def __init__(self, window, core):
        tfw.View.__init__(self, window, core)
        self.idle_id = None
        self.__build()

    def __build(self):
        '''Build the elements of this widget.'''
        self.player = tfw.Player()
        self.show_all()

    def shown(self, media):
        '''This view is shown. Start the video streaming.'''
        try:
            season = media.season
            episode = media.episode
        except AttributeError:
            season = episode = None
        """status = self.core.get_media_status(media.title_id, season, episode)
        if status.status == status.DOWNLOADED:
            self.start_streaming(media.title_id, season, episode)
        else:
            if status.status == status.MISSING:
                # Tell core to download the media
                self.download(media.title_id, season, episode)"""

    def start_streaming(title_id, season, episode):
        '''Start the streaming of the given title.'''
        pass

    def download(title_id, season, episode):
        '''Start downloading the media.'''
        # Tell the core to download the media
        # if error:
        #    Show a message and go back to the last view
        # else:
        #    loop:
        #        get the file status
        #        if error:
        #            Show a message and go back to the last view
        #        else:
        #            
        pass

    def key_pressed(self, widget, event):
        '''Show the player.'''
        self.player.show_all()
        self.idle_id = GLib.timeout_add_seconds(
            self.HIDE_PLAYER_SECONDS, self.hide_player)

    def hide_player(self):
        '''Hide the player.'''
        self.player.hide()

