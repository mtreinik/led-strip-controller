/*
 * utilities for converting hexadecimal strings to numbers
 */

uint32_t getHexDigit(uint32_t result, uint8_t digit) {
  int nextInt = digit - 48;
  if (nextInt > 9) {
    nextInt -= 39;
  }
  return (result << 4) + nextInt;
}

uint32_t getHexColor(int position, uint8_t line[]) {
  uint32_t result = 0;
  for (int i = 0; i < 6; i++) {
    result = getHexDigit(result, line[position + i]);
  }
  return result;
}

unsigned char getHexByte(int position, uint8_t line[]) {
  uint8_t result = getHexDigit(0, line[position]);
  result = getHexDigit(result, line[position + 1]);
  return result;
}

uint32_t mixColors(uint32_t color1, uint32_t color2, int weight1, int weight2, int shift) {
  uint32_t red1 = color1 >> 16;
  uint32_t red2 = color2 >> 16;
  int red = (red1 * weight1 + red2 * weight2) >> shift;

  uint32_t green1 = (color1 >> 8) & 0xff;
  uint32_t green2 = (color2 >> 8) & 0xff;
  int green = (green1 * weight1 + green2 * weight2) >> shift;

  uint32_t blue1 = color1 & 0xff;
  uint32_t blue2 = color2 & 0xff;
  int blue = (blue1 * weight1 + blue2 * weight2) >> shift;

  uint32_t color = red << 16 | green << 8 |  blue;
  return color;
}

