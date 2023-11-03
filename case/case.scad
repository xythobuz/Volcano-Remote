// https://www.waveshare.com/wiki/Pico-LCD-1.3
lcd_w = 52.0;
lcd_h = 26.5;

lcd_d = 1.0; // todo

// https://www.printables.com/model/210898-raspberry-pi-pico-case
use <pico_case.scad>
p_w = 21;
p_h = 51;
p_t = 1.0;

pico_header_h = 2.0; // todo
lcd_header_h = 10.0; // todo

header_h = pico_header_h + lcd_header_h;

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

hw();
