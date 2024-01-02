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

// https://www.waveshare.com/wiki/Pico-LCD-1.3
lcd_w = 52.0;
lcd_h = 26.5;

lcd_d = 1.0; // todo

// https://www.printables.com/model/210898-raspberry-pi-pico-case
use <pico_case.scad>
p_w = 21;
p_h = 51;
p_t = 1.0;
p_usb_h = 2.8;

pico_header_h = 2.5; // todo
lcd_header_h = 10.0; // todo

header_h = pico_header_h + lcd_header_h;

part="OPT_assembly"; //[bottom,top,button,print,OPT_assembly]
wall=1.5;
extra=4; //lip on the lid that presses top pcb down
thread=3;
air=0.5;
bissl=1/100;
button=4;
joystick=6;
height=16.8+air+extra;
width=52.4+1.5+2*air;
length=26.7+2*air;
$fa=1/1;
$fs=1/2;

total_width = width+2*wall;
total_len = length+4*wall+2*thread;

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
    cube([total_width, total_len, height + wall]);
    translate([wall,2*wall+thread,wall])cube([width,length,height+bissl]);
    for (tr=[[wall+thread/2,wall+thread/2,height/2+wall],
             [wall+thread/2,length+3*wall+1.5*thread,height/2+wall],
             [width+wall-thread/2,wall+thread/2,height/2+wall],
             [width+wall-thread/2,length+3*wall+1.5*thread,height/2+wall]])
      translate(tr) cylinder(h=height/2+bissl,d=thread);
    translate([wall/2,2*wall+thread+length/2,3-bissl])cube([wall+bissl,8,6],center=true);
    translate([0,2*wall+thread+length/2,wall+7])rotate([0,90,0])cylinder(d=button,h=2*wall+bissl,center=true);
  }
}

module top() {
  difference() {
    cube([total_width, total_len, wall]);
    for (tr=[[wall+thread/2,wall+thread/2,-bissl],
             [wall+thread/2,length+3*wall+1.5*thread,-bissl],
             [width+wall-thread/2,wall+thread/2,-bissl],
             [width+wall-thread/2,length+3*wall+1.5*thread,-bissl]])
      translate(tr) cylinder(h=wall+2*bissl,d=thread);
    translate([wall+1.5,2*wall+thread,-bissl])
      for (tr=[[47.7+air,5.5+0],
               [47.7+air,5.5+5.7],
               [47.7+air,5.5+2*5.7],
               [47.7+air,5.5+3*5.7],
               [6.7+0.5,13.8+air]])
        translate(tr) cylinder(h=wall+2*bissl,d=button);
    translate([wall+1.5+6.7+air,2*wall+thread+13.8+air,-bissl]) cylinder(h=wall+2*bissl,d=joystick);
    translate([wall+13.5+1.5,2*wall+thread,-bissl])cube([31,27,wall+2*bissl]);
  }
  translate([wall+13.5+1.5-2,2*wall+thread,-extra])cube([2,27,extra]);
  translate([wall+13.5+1.5+31,2*wall+thread,-extra])cube([2,27,extra]);
}

module button() {
  cylinder(h=1,d=5);
  cylinder(h=5,d=3);
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

            translate([-total_width / 2, -total_len / 2, height + wall])
            top();
        }

        translate([-total_width / 2 - 1, 0, -2])
        cube([total_width + 2, total_len / 2 + 2, height + wall + 4]);
    }
}
