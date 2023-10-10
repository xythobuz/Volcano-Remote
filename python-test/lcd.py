# https://www.waveshare.com/wiki/Pico-LCD-1.3
# https://thepihut.com/blogs/raspberry-pi-tutorials/coding-graphics-with-micropython-on-raspberry-pi-pico-displays
# https://api.arcade.academy/en/latest/_modules/arcade/draw_commands.html#draw_arc_filled

from machine import Pin, SPI, PWM
from array import array
import framebuf
import time
import os
import math
import gc

class KeyCheck:
    def __init__(self, new, old):
        self.new = new
        self.old = old

    def once(self, k):
        return self.new[k] and not self.old[k]

class LCD(framebuf.FrameBuffer):
    def __init__(self):
        self.pwm = PWM(Pin(13))
        self.pwm.freq(1000)
        self.brightness(0.0)

        self.width = 240
        self.height = 240

        self.cs = Pin(9, Pin.OUT)
        self.rst = Pin(12, Pin.OUT)

        self.cs(1)

        #self.spi = SPI(1)
        #self.spi = SPI(1,   1_000_000)
        self.spi = SPI(1, 100_000_000, polarity=0, phase=0, sck=Pin(10), mosi=Pin(11), miso=None)

        self.dc = Pin(8, Pin.OUT)
        self.dc(1)

        gc.collect()
        self.buffer = bytearray(self.height * self.width * 2)

        super().__init__(self.buffer, self.width, self.height, framebuf.RGB565)
        self.init_display()

        self.red    = self.color(0xFF, 0x00, 0x00)
        self.green  = self.color(0x00, 0xFF, 0x00)
        self.blue   = self.color(0x00, 0x00, 0xFF)
        self.yellow = self.color(0xFF, 0xFF, 0x00)
        self.white  = self.color(0xFF, 0xFF, 0xFF)
        self.black  = self.color(0x00, 0x00, 0x00)

        self.fill(self.black)
        self.show()

        self.keyA  = Pin(15, Pin.IN, Pin.PULL_UP)
        self.keyB  = Pin(17, Pin.IN, Pin.PULL_UP)
        self.keyX  = Pin(19, Pin.IN, Pin.PULL_UP)
        self.keyY  = Pin(21, Pin.IN, Pin.PULL_UP)
        self.up    = Pin( 2, Pin.IN, Pin.PULL_UP)
        self.down  = Pin(18, Pin.IN, Pin.PULL_UP)
        self.left  = Pin(16, Pin.IN, Pin.PULL_UP)
        self.right = Pin(20, Pin.IN, Pin.PULL_UP)
        self.ctrl  = Pin( 3, Pin.IN, Pin.PULL_UP)

        self.keys_old = {
            "a": False,
            "b": False,
            "x": False,
            "y": False,
            "up": False,
            "down": False,
            "left": False,
            "right": False,
            "enter": False,
        }

    def buttons(self):
        keys = {
            "a": self.keyA.value() == 0,
            "b": self.keyB.value() == 0,
            "x": self.keyX.value() == 0,
            "y": self.keyY.value() == 0,
            "up": self.up.value() == 0,
            "down": self.down.value() == 0,
            "left": self.left.value() == 0,
            "right": self.right.value() == 0,
            "enter": self.ctrl.value() == 0,
        }
        kc = KeyCheck(keys, self.keys_old)
        self.keys_old = keys.copy()
        return kc

    # Convert RGB888 to RGB565
    def color(self, R, G, B):
        return (((G & 0b00011100) << 3) + ((B & 0b11111000) >> 3) << 8) + (R & 0b11111000) + ((G & 0b11100000) >> 5)

    def brightness(self, v):
        self.pwm.duty_u16(int(v * 65535))

    def write_cmd(self, cmd):
        self.cs(1)
        self.dc(0)
        self.cs(0)
        self.spi.write(bytearray([cmd]))
        self.cs(1)

    def write_data(self, buf):
        self.cs(1)
        self.dc(1)
        self.cs(0)
        self.spi.write(bytearray([buf]))
        self.cs(1)

    def init_display(self):
        self.rst(1)
        self.rst(0)
        self.rst(1)
        self.write_cmd(0x36)
        self.write_data(0x70)
        self.write_cmd(0x3A)
        self.write_data(0x05)
        self.write_cmd(0xB2)
        self.write_data(0x0C)
        self.write_data(0x0C)
        self.write_data(0x00)
        self.write_data(0x33)
        self.write_data(0x33)
        self.write_cmd(0xB7)
        self.write_data(0x35)
        self.write_cmd(0xBB)
        self.write_data(0x19)
        self.write_cmd(0xC0)
        self.write_data(0x2C)
        self.write_cmd(0xC2)
        self.write_data(0x01)
        self.write_cmd(0xC3)
        self.write_data(0x12)
        self.write_cmd(0xC4)
        self.write_data(0x20)
        self.write_cmd(0xC6)
        self.write_data(0x0F)
        self.write_cmd(0xD0)
        self.write_data(0xA4)
        self.write_data(0xA1)
        self.write_cmd(0xE0)
        self.write_data(0xD0)
        self.write_data(0x04)
        self.write_data(0x0D)
        self.write_data(0x11)
        self.write_data(0x13)
        self.write_data(0x2B)
        self.write_data(0x3F)
        self.write_data(0x54)
        self.write_data(0x4C)
        self.write_data(0x18)
        self.write_data(0x0D)
        self.write_data(0x0B)
        self.write_data(0x1F)
        self.write_data(0x23)
        self.write_cmd(0xE1)
        self.write_data(0xD0)
        self.write_data(0x04)
        self.write_data(0x0C)
        self.write_data(0x11)
        self.write_data(0x13)
        self.write_data(0x2C)
        self.write_data(0x3F)
        self.write_data(0x44)
        self.write_data(0x51)
        self.write_data(0x2F)
        self.write_data(0x1F)
        self.write_data(0x1F)
        self.write_data(0x20)
        self.write_data(0x23)
        self.write_cmd(0x21)
        self.write_cmd(0x11)
        self.write_cmd(0x29)

    def show(self):
        self.write_cmd(0x2A)
        self.write_data(0x00)
        self.write_data(0x00)
        self.write_data(0x00)
        self.write_data(0xef)
        self.write_cmd(0x2B)
        self.write_data(0x00)
        self.write_data(0x00)
        self.write_data(0x00)
        self.write_data(0xEF)
        self.write_cmd(0x2C)
        self.cs(1)
        self.dc(1)
        self.cs(0)
        self.spi.write(self.buffer)
        self.cs(1)

    def circle(self, x, y, r, c):
        self.hline(x-r,y,r*2,c)
        for i in range(1,r):
            a = int(math.sqrt(r*r-i*i)) # Pythagoras!
            self.hline(x-a,y+i,a*2,c) # Lower half
            self.hline(x-a,y-i,a*2,c) # Upper half

    def ring(self, x, y, r, c):
        self.pixel(x-r,y,c)
        self.pixel(x+r,y,c)
        self.pixel(x,y-r,c)
        self.pixel(x,y+r,c)
        for i in range(1, r):
            a = int(math.sqrt(r*r-i*i))
            self.pixel(x-a,y-i,c)
            self.pixel(x+a,y-i,c)
            self.pixel(x-a,y+i,c)
            self.pixel(x+a,y+i,c)
            self.pixel(x-i,y-a,c)
            self.pixel(x+i,y-a,c)
            self.pixel(x-i,y+a,c)
            self.pixel(x+i,y+a,c)

    def arc(self, x_off, y_off, w, h, c,
                   start_angle, end_angle,
                   filled = True,
                   num_segments = 64):
        start_segment = int(start_angle / 360 * num_segments)
        end_segment = int(end_angle / 360 * num_segments)

        gc.collect()
        point_list = array('h')
        point_list.append(0)
        point_list.append(0)

        for segment in range(start_segment, end_segment + 1):
            theta = 2.0 * 3.1415926 * segment / num_segments
            x = w * math.cos(theta) / 2
            y = h * math.sin(theta) / 2

            i = (segment - start_segment + 1) * 2
            point_list.append(int(x))
            point_list.append(int(y))

        self.poly(int(x_off), int(y_off), point_list, c, filled)

    def pie(self, x0, y0, w, c_border, c_circle, v):
        if v > 0.0:
            self.arc(int(x0), int(y0), int(w), int(w), c_circle, -90, int(v * 360) - 90)
        self.ring(int(x0), int(y0), int(w / 2), c_border)

    def textC(self, s, x, y, c, bgColor = None):
        xStart = x - int(len(s) * 8 / 2)
        yStart = y - 5

        if bgColor != None:
            self.rect(xStart, yStart - 1, len(s) * 8, 10, bgColor, True)

        self.text(s, xStart, yStart, c)

    def textLine(self, s, c, off = 0):
        charsPerLine = int(self.width / 8)
        lines = list(s[0+i:charsPerLine+i] for i in range(0, len(s), charsPerLine))
        n = 0
        for i, l in enumerate(lines):
            self.text(l, 0, (i + off) * 10, c)
            n += 1
            if i >= (self.height / 10):
                break
        return n

    def textBlock(self, s, c):
        lines = s.split("\n")
        off = 0
        for l in lines:
            off += self.textLine(l, c, off)
