
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

#TODO: Set max size for cache

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


PICTURE_MAX_RETRIES = 5

class Core(object):
    '''The application's core.'''

    def __init__(self, server_address):
        self.server = tvfamilyapi.Server(server_address)
        self.profile = None
        self.downloading_pictures = {}
        self.pictures_lock = threading.Lock()

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
        return self._get_picture(media.poster_url)

    def _get_picture(self, url):
        '''Get a picture from its url.'''
        path = os.path.join(tfp.get_cache_path(), url.rpartition('/')[2])
        # Check if this picture is already being downloaded
        has_condition, cond = self._get_picture_condition(path)
        if has_condition:
            self._wait_for_picture(path, cond)
        else:
            self._download_picture(url, path, cond)
        return path

    def _get_picture_condition(self, path):
        '''Return True if a condition variable for the given picture exists and
        the condition itself, or a False and a new condition.
        '''
        with self.pictures_lock:
            try:
                condition = self.downloading_pictures[path]
                has_condition = True
            except KeyError:
                condition = threading.Condition()
                self.downloading_pictures[path] = condition
                has_condition = False
        return has_condition, condition

    def _wait_for_picture(self, path, condition):
        '''Wait for a picture to be downloaded by another thread.'''
        with condition:
            condition.wait()
        # Check if indeed the poster has been downloaded
        if not os.path.exists(path):
            raise IOError('no picture downloaded')

    def _download_picture(self, url, path, condition):
        '''Download the poster.'''
        # Check if the picture is already downloaded
        if not os.path.exists(path):
            count = 0
            while count < PICTURE_MAX_RETRIES:
                count += 1
                try:
                    with open(path, 'wb') as f:
                        c = pycurl.Curl()
                        c.setopt(c.URL, url)
                        c.setopt(c.WRITEFUNCTION, f.write)
                        c.perform()
                        c.close()
                    break
                except Exception as e:
                    last_error = e
            if count == PICTURE_MAX_RETRIES:
                # The picture couldn't be downloaded. Make sure there is not a
                # partial file
                try:
                    os.unlink(path)
                except OSError: pass
                self._remove_condition(path, condition)
                raise last_error
        self._remove_condition(path, condition)

    def _remove_condition(self, path, condition):
        '''Remove the condition from the dictionary.'''
        with self.pictures_lock:
            del self.downloading_pictures[path]
        with condition:
            condition.notify_all()

    def get_title(self, title_id):
        '''Get the information of a title.'''
        return self.server.get_title(title_id)

    def get_still(self, episode):
        '''Get an episode's still.'''
        return self._get_picture(episode['still'])

    def get_media_status(self, title_id, season, episode):
        '''Return the video status of a given media.'''
        return self.server.get_media_status(title_id, season, episode)

