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

  uint32_t color = red << 16 | green << 8 | blue;
  return color;
}

