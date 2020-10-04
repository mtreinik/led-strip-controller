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

uint32_t getHexColor(int &pos, uint8_t line[]) {
  uint32_t result = 0;
  for (int i = 0; i < 6; i++) {
    result = getHexDigit(result, line[pos++]);
  }
  return result;
}

unsigned char getHexByte(int &pos, uint8_t line[]) {
  uint8_t result = getHexDigit(0, line[pos++]);
  result = getHexDigit(result, line[pos++]);
  return result;
}

