
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

from tvfamilygtk import paths as tfp
import tvfamilyapi

__author__ = 'Antonio Serrano Hernandez'
__copyright__ = 'Copyright (C) 2018 2019 Antonio Serrano Hernandez'
__version__ = '0.1'
__license__ = 'GPL'
__maintainer__ = 'Antonio Serrano Hernandez'
__email__ = 'toni.serranoh@gmail.com'
__status__ = 'Development'
__homepage__ = 'https://github.com/aserranoh/tvfamily-gtk'


POSTER_MAX_RETRIES = 5

class Core(object):
    '''The application's core.'''

    def __init__(self, server_address):
        self.server = tvfamilyapi.Server(server_address)
        self.profile = None
        self.downloading_posters = {}
        self.posters_lock = threading.Lock()

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
        path = os.path.join(
            tfp.get_cache_path(), media.poster_url.rpartition('/')[2])
        # Check if this poster is already being downloaded
        has_condition, cond = self._get_poster_condition(path)
        if has_condition:
            self._wait_for_poster(path, cond)
        else:
            self._download_poster(path, cond)
        return path

    def _get_poster_condition(self, path):
        '''Return True if a condition variable for the given poster exists and
        the condition itself, or a False and a new condition.
        '''
        with self.posters_lock:
            try:
                condition = self.downloading_posters[path]
                has_condition = True
            except KeyError:
                condition = threading.Condition()
                self.downloading_posters[path] = condition
                has_condition = False
        return has_condition, condition

    def _wait_for_poster(self, path, condition):
        '''Wait for a poster to be downloaded by another thread.'''
        with condition:
            condition.wait()
        # Check if indeed the poster has been downloaded
        if not os.path.exists(path):
            raise IOError('no poster downloaded')

    def _download_poster(self, path, condition):
        '''Download the poster.'''
        # Check if the poster is already downloaded
        if not os.path.exists(path):
            count = 0
            while count < POSTER_MAX_RETRIES:
                count += 1
                try:
                    with open(path, 'wb') as f:
                        c = pycurl.Curl()
                        c.setopt(c.URL, media.poster_url)
                        c.setopt(c.WRITEFUNCTION, f.write)
                        c.perform()
                        c.close()
                    break
                except Exception as e:
                    last_error = e
            if count == POSTER_MAX_RETRIES:
                # The poster couldn't be downloaded. Make sure there is not a
                # partial file
                try:
                    os.unlink(path)
                except OSError: pass
                self._remove_condition(path, condition)
                raise last_error
        self._remove_condition(path, condition)

    def _remove_condition(self, path, condition):
        '''Remove the condition from the dictionary.'''
        with self.posters_lock:
            del self.downloading_posters[path]
        with condition:
            condition.notify_all()

    def get_title(self, title_id):
        '''Get the information of a title.'''
        return self.server.get_title(title_id)

