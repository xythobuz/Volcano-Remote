/*
 * case.scad
 *
 * https://github.com/kauzerei/OpensCadaver/blob/main/models/pico_stuff.scad
 *
 * Copyright (c) 2023 Kauzerei (openautolab@kauzerei.de)
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

// prev [bottom,top,button,print,OPT_assembly]
part = "OPT_assembly"; //[bottom,OPT_assembly]
wall = 1.5;

// https://www.waveshare.com/wiki/Pico-LCD-1.3
lcd_w = 52.0;
lcd_h = 26.5;
lcd_d = 1.55;

// https://www.printables.com/model/210898-raspberry-pi-pico-case
use <pico_case.scad>
p_w = 21;
p_h = 51;
p_t = 1.0;
p_usb_h = 2.8;

usb_cut_w = 8.5;
usb_cut_h = p_usb_h + 0.4;

pico_header_h = 2.6;
lcd_header_h = 9.0;

pico_slot_w = p_w + 1.2;
pico_slot_h = p_h + 1.6;

extra=4; //lip on the lid that presses top pcb down
thread=3;
air=0.5;
bissl=1/100;
joystick=6;
$fa=1/1;
$fs=1/2;

add_len = 0.7 + 1; // + 2 * wall + thread;
button = 4;
button_hole = button + 1.0;
button_stem = button - 0.4;
button_cut_side = 0.6;

header_h = pico_header_h + lcd_header_h;

length = pico_slot_w; //26.7+2*air;
width = pico_slot_h; //52.4+1.5+2*air;
height = p_t + p_usb_h + header_h; //16.8+air+extra;

total_width = width + 2 * wall;
total_len = length + 2 * wall;

pwr_w = 5.0;
pwr_h = 10.0;
pwr_gap = 0.3;
pwr_wg = pwr_w + 2 * pwr_gap;
pwr_hg = pwr_h + 3 * pwr_gap;

module lcd() {
    color("blue")
    translate([-lcd_h / 2, -lcd_w / 2, 0])
    cube([lcd_h, lcd_w, lcd_d]);
}

module hw() {
    translate([0, 0, header_h])
    lcd();

    translate([p_w / 2, -p_h / 2, 0])
    rotate([0, 180, 0])
    pico();
}

module bottom() {
    difference() {
        translate([0, -add_len, 0])
        cube([total_width, total_len + 2 * add_len, height + wall]);

        translate([wall, wall, wall])
        cube([width, length, height + 0.1]);

        // screws
        if (0)
        #for (tr=[[wall+thread/2,wall+thread/2,height/2+wall],
             [wall+thread/2,length+3*wall+1.5*thread,height/2+wall],
             [width+wall-thread/2,wall+thread/2,height/2+wall],
             [width+wall-thread/2,length+3*wall+1.5*thread,height/2+wall]])
        translate(tr) cylinder(h=height/2+bissl,d=thread);

        // usb cutout
        translate([wall / 2, wall + length / 2, usb_cut_h / 2 + wall])
        cube([wall + bissl, usb_cut_w, usb_cut_h], center = true);

        // power button cutout
        translate([-0.1, wall + length / 2 - pwr_wg / 2, wall + usb_cut_h - 0.1])
        cube([wall + 0.2, pwr_wg, pwr_h + 0.1]);

        // bootsel button
        translate([14.475, 16.05, -0.1])
        cylinder(d = 4.5, h = wall + 0.2);
    }

    // power button
    translate([0, wall + length / 2 - pwr_w / 2, wall + usb_cut_h + (pwr_hg - pwr_h)])
    cube([wall, pwr_w, pwr_h + 0.1]);

    // pico screw posts
    translate([total_width / 2, total_len / 2, wall])
    for (x = [-p_h / 2 + 2, p_h / 2 - 2])
    for (y = [-p_w / 2 + 4.8, p_w / 2 - 4.8])
    translate([x, y, 0])
    difference() {
        cylinder(d = 4, h = p_usb_h);

        translate([0, 0, -0.1])
        cylinder(d = 2.2, h = p_usb_h + 0.2);
    }
}

module top() {
    difference() {
        cube([total_width, total_len, wall]);
        for (tr=[[wall+thread/2,wall+thread/2,-bissl],
             [wall+thread/2,length+3*wall+1.5*thread,-bissl],
             [width+wall-thread/2,wall+thread/2,-bissl],
             [width+wall-thread/2,length+3*wall+1.5*thread,-bissl]])
        translate(tr)
        cylinder(h=wall+2*bissl,d=thread);

        translate([wall+1.5,2*wall+thread,-bissl])
        for (i = [0 : 3])
        translate([47.7 + air, 5.5 + i * 5.7])
        cylinder(h=wall+2*bissl,d=button);

        translate([wall+1.5+6.7+air,2*wall+thread+13.8+air,-bissl])
        cylinder(h=wall+2*bissl,d=joystick);

        translate([wall+13.5+1.5,2*wall+thread,-bissl])
        cube([31,27,wall+2*bissl]);
    }

    translate([wall+13.5+1.5-1.5,2*wall+thread,-extra])
    cube([1.5,27,extra]);

    translate([wall+13.5+1.5+31,2*wall+thread,-extra])
    cube([1.5,27,extra]);
}

module button() {
    difference() {
        cylinder(h = 1, d = button_hole);

        translate([button_hole / 2 * -3 + button_cut_side, -button_hole / 2 - 0.1, -0.1])
        cube([button_hole, button_hole + 0.2, 1.2]);
    }

    cylinder(h = 4, d = button_stem);
}

module bottom_assm() {
    %translate([0, 0, wall + p_t + p_usb_h])
    rotate([0, 0, 90])
    hw();

    translate([-total_width / 2, -total_len / 2, 0])
    bottom();
}

if (part == "bottom") {
    bottom_assm();
} else if (part == "top") {
    rotate([180, 0, 0])
    top();
} else if (part == "button") {
    button();
} else if (part == "print"){
    translate([0, 10, 0])
    bottom();

    translate([0, -10, wall])
    rotate([180, 0, 0])
    top();

    for (i = [1 : 4])
    translate([-10, -25 + i * 10, 0])
    button();
} else {
    difference() {
        union() {
            bottom_assm();

            if (0)
            translate([-total_width / 2, -total_len / 2, height + wall])
            top();

            if (0)
            for (i = [0 : 3])
            translate([47.7 + air, 5.5 + i * 5.7])
            translate([wall + 1.5, 2 * wall + thread, -1])
            translate([-total_width / 2, -total_len / 2, height + wall])
            button();
        }

        if (1)
        translate([-total_width / 2 - 1, 0, -2])
        cube([total_width + 2, (total_len + 2 * add_len) / 2 + 2, height + wall + 10]);
    }
}
