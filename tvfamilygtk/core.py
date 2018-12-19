
'''core.py - Application's core.

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

import threading

import tvfamilyapi

__author__ = 'Antonio Serrano Hernandez'
__copyright__ = 'Copyright (C) 2018 Antonio Serrano Hernandez'
__version__ = '0.1'
__license__ = 'GPL'
__maintainer__ = 'Antonio Serrano Hernandez'
__email__ = 'toni.serranoh@gmail.com'
__status__ = 'Development'
__homepage__ = 'https://github.com/aserranoh/tvfamily-gtk'


class Core(object):
    '''The application's core.'''

    def __init__(self, server_address):
        self.server = tvfamilyapi.Server(server_address)
        self.profile = None

    def request_profiles_list(self, callback):
        '''Request the profiles list to the server and when done, send the
        list back using the given callback.
        '''
        t = threading.Thread(
            target=self._request_profiles_list_thread, args=(callback,))
        t.start()

    def _request_profiles_list_thread(self, callback):
        '''Get the list of profiles in a parallel thread.'''
        try:
            callback(profiles=self.server.get_profiles())
        except tvfamilyapi.ServiceError as e:
            callback(error=e)

    def request_profile_picture(self, name, callback):
        '''Request the picture for the given profile and when done, send the
        image back using the given callback.
        '''
        t = threading.Thread(
            target=self._request_profile_picture_thread, args=(name, callback))
        t.start()

    def _request_profile_picture_thread(self, name, callback):
        '''Get the picture for the given profile in a parallel thread.'''
        try:
            callback(name, picture=self.server.get_profile_picture(name))
        except tvfamilyapi.ServiceError as e:
            callback(name, error=e)

    def request_create_profile(self, name, picture=None, callback=None):
        '''Request the creation of a profile.'''
        t = threading.Thread(target=self._request_create_profile_thread,
            args=(name, picture, callback))
        t.start()

    def _request_create_profile_thread(self, name, picture, callback):
        '''Create the profile in a parallel thread.'''
        try:
            self.server.create_profile(name, picture)
            callback(name)
        except tvfamilyapi.ServiceError as e:
            callback(name, e)

    def request_set_profile_picture(self, picture=None, callback=None):
        '''Request the update of the profile picture.'''
        t = threading.Thread(target=self._request_set_profile_picture_thread,
            args=(self.profile, picture, callback))
        t.start()

    def _request_set_profile_picture_thread(self, name, picture, callback):
        '''Set the profile picture in a parallel thread.'''
        try:
            self.server.set_profile_picture(name, picture)
            callback(name)
        except tvfamilyapi.ServiceError as e:
            callback(name, e)

    def request_delete_profile(self, callback):
        '''Request the deletion of the current profile.'''
        t = threading.Thread(target=self._request_delete_profile_thread,
            args=(self.profile, callback))
        t.start()

    def _request_delete_profile_thread(self, name, callback):
        '''Delete the profile in a parallel thread.'''
        try:
            self.server.delete_profile(name)
            callback(name)
        except tvfamilyapi.ServiceError as e:
            callback(name, e)

    def set_profile(self, name):
        '''Set the current profile.'''
        self.profile = name

    def get_profile(self):
        '''Return the current profile.'''
        return self.profile

    def request_categories(self, callback):
        '''Request the categories list to the server and when done, send the
        list back using the given callback.
        '''
        t = threading.Thread(
            target=self._request_categories_thread, args=(callback,))
        t.start()

    def _request_categories_thread(self, callback):
        '''Get the list of categories in a parallel thread.'''
        try:
            callback(categories=self.server.get_categories())
        except tvfamilyapi.ServiceError as e:
            callback(error=e)

