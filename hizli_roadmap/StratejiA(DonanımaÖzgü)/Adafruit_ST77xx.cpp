/*!
 * @file Adafruit_ST77xx.cpp
 * ATMEGA2560-specific. Direct register SPI. No multi-platform code.
 */

#include "Adafruit_ST77xx.h"

Adafruit_ST77xx::Adafruit_ST77xx(uint16_t w, uint16_t h, int8_t rst)
    : Adafruit_SPITFT(w, h, rst) {}

void Adafruit_ST77xx::begin(uint32_t freq) {
  (void)freq;
  initSPI();
}

void Adafruit_ST77xx::commonInit(const uint8_t *cmdList) {
  begin(0);
  displayInit(cmdList);
}

void Adafruit_ST77xx::displayInit(const uint8_t *addr) {
  uint8_t numCommands = pgm_read_byte(addr++);
  while (numCommands--) {
    uint8_t cmd = pgm_read_byte(addr++);
    uint8_t numArgs = pgm_read_byte(addr++);
    uint8_t ms = numArgs & ST_CMD_DELAY;
    numArgs &= ~ST_CMD_DELAY;

    sendCommand(cmd, addr, numArgs);
    addr += numArgs;

    if (ms) {
      ms = pgm_read_byte(addr++);
      delay(ms == 255 ? 500 : ms);
    }
  }
}

void Adafruit_ST77xx::setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                                    uint16_t h) {
  x += _xstart;
  y += _ystart;

  uint8_t caset[4] = {(uint8_t)(x >> 8), (uint8_t)x,
                      (uint8_t)((x + w - 1) >> 8), (uint8_t)(x + w - 1)};
  uint8_t raset[4] = {(uint8_t)(y >> 8), (uint8_t)y,
                      (uint8_t)((y + h - 1) >> 8), (uint8_t)(y + h - 1)};

  sendCommand(ST77XX_CASET, caset, 4);
  sendCommand(ST77XX_RASET, raset, 4);
  writeCommand(ST77XX_RAMWR);
}

void Adafruit_ST77xx::setColRowStart(int8_t col, int8_t row) {
  _colstart = col;
  _rowstart = row;
}

void Adafruit_ST77xx::enableDisplay(boolean enable) {
  startWrite();
  writeCommand(enable ? ST77XX_DISPON : ST77XX_DISPOFF);
  endWrite();
}

void Adafruit_ST77xx::enableTearing(boolean enable) {
  startWrite();
  writeCommand(enable ? ST77XX_TEON : ST77XX_TEOFF);
  endWrite();
}

void Adafruit_ST77xx::enableSleep(boolean enable) {
  startWrite();
  writeCommand(enable ? ST77XX_SLPIN : ST77XX_SLPOUT);
  endWrite();
}

void Adafruit_ST77xx::setRotation(uint8_t r) {
  // Subclass (ST7735) overrides this with tab-specific logic
  Adafruit_GFX::setRotation(r);
}
