#!/usr/bin/env python

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
