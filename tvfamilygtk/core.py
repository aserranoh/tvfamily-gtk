
'''core.py - Application's core.

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

import os
import pycurl
import threading

import tvfamilyapi

__author__ = 'Antonio Serrano Hernandez'
__copyright__ = 'Copyright (C) 2018 2019 Antonio Serrano Hernandez'
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

    def get_profiles(self):
        '''Return the profiles list. '''
        return self.server.get_profiles()

    def get_profile_picture(self, name):
        '''Return the picture for the given profile.'''
        return self.server.get_profile_picture(name)

    def create_profile(self, name, picture=None):
        '''Create a profile.'''
        self.server.create_profile(name, picture)

    def set_profile_picture(self, picture=None):
        '''Update the picture of the current profile.'''
        self.server.set_profile_picture(self.profile, picture)

    def delete_profile(self):
        '''Delete the current profile.'''
        self.server.delete_profile(self.profile)

    def set_profile(self, name):
        '''Set the current profile.'''
        self.profile = name

    def get_profile(self):
        '''Return the current profile.'''
        return self.profile

    def get_categories(self):
        '''Return the categories list.'''
        return self.server.get_categories()

    def get_medias(self, category):
        '''Get the list of medias of a given category.'''
        return self.server.get_top(self.profile, category)

    def get_poster(self, media):
        '''Get a poster from its url.'''
        path = os.path.join('cache', media.poster_url.rpartition('/')[2])
        if not os.path.exists(path):
            with open(path, 'wb') as f:
                c = pycurl.Curl()
                c.setopt(c.URL, media.poster_url)
                c.setopt(c.WRITEFUNCTION, f.write)
                c.perform()
                c.close()
        return path

    def request_title(self, title_id, callback):
        '''Get the information of a title.'''
        t = threading.Thread(
            target=self._request_title_thread, args=(title_id, callback))
        t.start()

    def _request_title_thread(self, title_id, callback):
        '''Get the title information in a parallel thread.'''
        try:
            callback(title=self.server.get_title(title_id))
        except tvfamilyapi.ServiceError as e:
            callback(error=e)

