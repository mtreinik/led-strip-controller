# Web controller for RGB led strip

## Software

### Client

### API

The led API can receive a number of commands. Parameters for commands
are given as two-digit hex numbers.

Commands are sent as query parameters for a HTTP GET request to the
server. For example `http://10.0.0.80/?s808000`.


| Command | Description | Format | Example | Explanation of example |
|---------|-------------|--------|---------|------------------------|
| a       | set colors from an array | a[number of pixels][r1][g1][b1][r2][g2][b2]... | a03ff0000ffff0000ff00 | set three colors: red, yellow and green |
| s       | set all pixels to a single color | s[r][g][b] | sff00ff | set all pixels to bright magenta |
| c       | clear all pixels | c | turns off all pixels |
| p       | set a single pixel | p[position][r][g][b] | p28ffffff | set pixel number 40 (0x28 in hex) to white |
| m       | draw a moving sprite | m[background r][background g][background b][size of sprite][sprite position][sprite speed][r1][g1][b1][r2][g2][b2]... | m000000040084000500003000006000ffffff | animate a white pixel with green fading tail quite slowly slowly on a black background |

## Electronics

The led strip is an [Adafruit DotStar Digital LED
Strip](https://www.adafruit.com/product/2239?length=4) that has 240
individually-addressable RGB LEDs. It is controlled by an [Adafruit
Feather M0 WiFi - ATSAMD21 +
ATWINC1500](https://www.adafruit.com/product/3010) Arduino board with
WiFi capability.

The led strip is powered by a 5V 10A switching power supply.

### Pinouts used on the controller

| Pin | Description           |Wire color | 
|-----|-----------------------|-----------|
|GND  | ground                | black     |
|USB  | 5V power              | red       |
|5    | data to led strip     | green     |
|6    | clock to led strip    | yellow    |

