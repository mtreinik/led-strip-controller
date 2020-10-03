# Web controller for RGB led strip

## 

## Software

### API

The led API can receive a number of commands. Parameters for commands
are given as two-digit hex numbers.

Commands are sent as query parameters for a HTTP GET request to the
server. For example `http://<hostname>/?s808000`.


| Command | Description | Format | Example | Explanation of example |
|---------|-------------|--------|---------|------------------------|
| c       | clear all pixels | c | c | turns off all pixels |
| s       | set all pixels to a single color | s[r][g][b] | sff00ff | set all pixels to bright magenta |
| p       | draw a single pixel | p[position][r][g][b] | p28ffffff | set pixel number 40 (0x28 in hex) to white |
| a       | set colors from an array | a[number of pixels][r1][g1][b1][r2][g2][b2]... | a03ff0000ffff0000ff00 | set three colors: red, yellow and green |
| g       | draw a color gradient | g[start position][end position][r1][g1][b1][r2][g2][b2] | g0010ff0000ffff00 | draw a 16 pixel gradient from red to yellow at the start of the led strip |
| m       | draw a moving sprite | m[background r][background g][background b][size of sprite][sprite position][sprite speed][r1][g1][b1][r2][g2][b2]... | m000000040084000500003000006000ffffff | animate a white pixel with green fading tail quite slowly slowly on a black background |
| f       | animate flames        | f[fuel amount][damping amount] | f2f10 | 

## Electronics

* LEDs: 4 meter [Adafruit DotStar Digital LED
Strip](https://www.adafruit.com/product/2239?length=4) that has 240
individually-addressable RGB LEDs
* microcontroller: [Adafruit
Feather M0 WiFi - ATSAMD21 +
ATWINC1500](https://www.adafruit.com/product/3010) Arduino board with
WiFi capability.
* power supply: 5V 10A switching power supply for the LEDs

### Schematic

Here is a schematic of how the power supply (on the left),
microcontroller and LED strip (on the right) are connected:

![Schematic of the control circuitry](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/images/schematic.png)

### Pins used on the controller

| Pin | Description           |Wire color | 
|-----|-----------------------|-----------|
|GND  | ground                | black     |
|VBUS | 5V power              | red       |
|5    | data to led strip     | green     |
|6    | clock to led strip    | yellow    |

![Feather M0 annotated with pins in use](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/images/feather.png)

[Feather M0 pinouts without annotations](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/images/feather_m0_wifi_pinout_v1.2-1.png)

## Enclosure

Here is everything put together in the enclosure:

![Finished enclosure with all electronics](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/images/finished-annotated.jpg)

[Image of finished enclosure without annotations](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/images/finished.jpg)

There is a Micro USB connector for flashing the microcontroller. I
added a power cord that goes to the other end of the LED strip to make
sure that all leds receive power evenly.

The enclosure is 3D printed and has a sliding lid.

![Enclosure for LED controller electronics](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/images/enclosure.png)

Here are the 3D models of the bottom and lid of the enclosure:

- [led_strip_controller_enclosure_bottom.stl](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/models/led_strip_controller_enclosure_bottom.stl)
- [led_strip_controller_enclosure_lid.stl](https://raw.githubusercontent.com/mtreinik/led-strip-controller/main/models/led_strip_controller_enclosure_lid.stl)
