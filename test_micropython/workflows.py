#!/usr/bin/env python3

# ----------------------------------------------------------------------------
# Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# See <http://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------------

workflows = [
    {
        "name": "Default",
        "author": "xythobuz",
        "steps": [
            (185.0, 15.0, 5.0),
            (195.0, 10.0, 20.0),
            (205.0, 10.0, 20.0),
        ],
        "notify": (4, 1.0),
        "reset_temperature": 190.0,
    },
    {
        "name": "Relaxo",
        "author": "xythobuz",
        "steps": [
            (175.0, 15.0, 5.0),
            (185.0, 10.0, 20.0),
            (195.0, 10.0, 20.0),
        ],
        "notify": (4, 1.0),
        "reset_temperature": 190.0,
    },
    {
        "name": "Hardcore",
        "author": "xythobuz",
        "steps": [
            (190.0, 15.0, 5.0),
            (205.0, 10.0, 20.0),
            (220.0, 10.0, 20.0),
        ],
        "notify": (4, 1.0),
        "reset_temperature": 190.0,
    },
    {
        "name": "Vorbi",
        "author": "Rinor",
        "steps": [
            (176.0, 10.0, 6.0),
            (187.0, 5.0, 10.0),
            (204.0, 3.0, 10.0),
            (217.0, 5.0, 10.0),
        ],
        "notify": None,
        "reset_temperature": None,
    },
]
