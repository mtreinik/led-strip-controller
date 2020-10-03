#define GAMMA 0.36
#define GAMMA_VALUES 256
int gammaTable[GAMMA_VALUES]= {};
int inverseGammaTable[GAMMA_VALUES] = {};

int roundToInt(double val) {
  return (int) (val + 0.5);
}

void initializeGammaTables() {
  for (int i = 0; i < GAMMA_VALUES; i++) {
    gammaTable[i] = roundToInt(pow((double) i / GAMMA_VALUES, 2.2) * GAMMA_VALUES);
    inverseGammaTable[i] = roundToInt(pow((double) i / GAMMA_VALUES, 1/2.2) * GAMMA_VALUES);
  }
}

uint32_t mixColors(uint32_t color1, uint32_t color2, int weight1, int weight2, int shift) {
  uint32_t red1 = color1 >> 16;
  uint32_t red2 = color2 >> 16;
  int red = (gammaTable[red1] * weight1 + gammaTable[red2] * weight2) >> shift;

  uint32_t green1 = (color1 >> 8) & 0xff;
  uint32_t green2 = (color2 >> 8) & 0xff;
  int green = (gammaTable[green1] * weight1 + gammaTable[green2] * weight2) >> shift;

  uint32_t blue1 = color1 & 0xff;
  uint32_t blue2 = color2 & 0xff;
  int blue = (gammaTable[blue1] * weight1 + gammaTable[blue2] * weight2) >> shift;

  uint32_t color = inverseGammaTable[red] << 16 | inverseGammaTable[green] << 8 | inverseGammaTable[blue];
  return color;
}

