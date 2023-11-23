/*
 * workflow_default.c
 *
 * Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "workflow.h"

#define NOTIFY \
    { .op = OP_WAIT_TIME, .val = 1000 }, \
    { .op = OP_PUMP_TIME, .val = 1000 }

const uint16_t wf_default_count = 5;

const struct workflow wf_default_data[] = {
    {
        .name = "XXL",
        .author = "xythobuz",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1850 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 8000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1950 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 25000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2050 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 25000 },

            NOTIFY, NOTIFY, NOTIFY, NOTIFY,

            { .op = OP_SET_TEMPERATURE, .val = 1900 },
        },
        .count = 18,
    }, {
        .name = "Default",
        .author = "xythobuz",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1850 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 5000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1950 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2050 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            NOTIFY, NOTIFY, NOTIFY, NOTIFY,

            { .op = OP_SET_TEMPERATURE, .val = 1900 },
        },
        .count = 18,
    }, {
        .name = "Vorbi",
        .author = "Rinor",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1760 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 6000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1870 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 10000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2040 },
            { .op = OP_WAIT_TIME, .val = 3000 },
            { .op = OP_PUMP_TIME, .val = 10000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2170 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 10000 },
        },
        .count = 12,
    }, {
        .name = "Relaxo",
        .author = "xythobuz",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1750 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 5000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1850 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 1950 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            NOTIFY, NOTIFY, NOTIFY, NOTIFY,

            { .op = OP_SET_TEMPERATURE, .val = 1900 },
        },
        .count = 18,
    }, {
        .name = "Hotty",
        .author = "xythobuz",
        .steps = {
            { .op = OP_WAIT_TEMPERATURE, .val = 1900 },
            { .op = OP_WAIT_TIME, .val = 10000 },
            { .op = OP_PUMP_TIME, .val = 5000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2050 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            { .op = OP_WAIT_TEMPERATURE, .val = 2200 },
            { .op = OP_WAIT_TIME, .val = 5000 },
            { .op = OP_PUMP_TIME, .val = 20000 },

            NOTIFY, NOTIFY, NOTIFY, NOTIFY,

            { .op = OP_SET_TEMPERATURE, .val = 1900 },
        },
        .count = 18,
    },
};
