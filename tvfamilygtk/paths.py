
'''paths.py - Various paths and filenames.

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

# TODO: final static path (invalid address)

import os

__author__ = 'Antonio Serrano Hernandez'
__copyright__ = 'Copyright (C) 2018 2019 Antonio Serrano Hernandez'
__version__ = '0.1'
__license__ = 'GPL'
__maintainer__ = 'Antonio Serrano Hernandez'
__email__ = 'toni.serranoh@gmail.com'
__status__ = 'Development'
__homepage__ = 'https://github.com/aserranoh/tvfamily-gtk'


STATIC_PATH = 'data'
DEFAULT_PICTURE = 'profile-default.svg'
STYLES_FILE = 'styles.css'
LOGO_FILE = 'tvfamily.svg'

def get_default_picture():
    return os.path.join(STATIC_PATH, DEFAULT_PICTURE)

def get_styles():
    return os.path.join(STATIC_PATH, STYLES_FILE)

def get_logo():
    return os.path.join(STATIC_PATH, LOGO_FILE)

def get_image(image):
    return os.path.join(STATIC_PATH, image + '.svg')

