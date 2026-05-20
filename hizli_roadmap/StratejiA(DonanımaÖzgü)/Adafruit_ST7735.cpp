/*!
 * @file Adafruit_ST7735.cpp
 * ATMEGA2560-specific. Single constructor (no CS/DC params, hardcoded).
 * SPIClass*, soft SPI constructors removed. ESP8266 guards removed.
 */

#include "Adafruit_ST7735.h"
#include "Adafruit_ST77xx.h"

// Constructor — CS/DC ignored, hardcoded in SPITFT
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_ST77xx(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, rst) {}

Adafruit_ST7735::Adafruit_ST7735(int8_t rst)
    : Adafruit_ST77xx(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, rst) {}

// clang-format off
static const uint8_t PROGMEM
  Bcmd[] = {
    18,
    ST77XX_SWRESET,   ST_CMD_DELAY,
      50,
    ST77XX_SLPOUT,    ST_CMD_DELAY,
      255,
    ST77XX_COLMOD,  1+ST_CMD_DELAY,
      0x05,
      10,
    ST7735_FRMCTR1, 3+ST_CMD_DELAY,
      0x00, 0x06, 0x03,
      10,
    ST77XX_MADCTL,  1,
      0x08,
    ST7735_DISSET5, 2,
      0x15, 0x02,
    ST7735_INVCTR,  1,
      0x0,
    ST7735_PWCTR1,  2+ST_CMD_DELAY,
      0x02, 0x70,
      10,
    ST7735_PWCTR2,  1,
      0x05,
    ST7735_PWCTR3,  2,
      0x01, 0x02,
    ST7735_VMCTR1,  2+ST_CMD_DELAY,
      0x3C, 0x38,
      10,
    ST7735_PWCTR6,  2,
      0x11, 0x15,
    ST7735_GMCTRP1,16,
      0x09, 0x16, 0x09, 0x20,
      0x21, 0x1B, 0x13, 0x19,
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1,16+ST_CMD_DELAY,
      0x0B, 0x14, 0x08, 0x1E,
      0x22, 0x1D, 0x18, 0x1E,
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,
    ST77XX_CASET,   4,
      0x00, 0x02, 0x00, 0x81,
    ST77XX_RASET,   4,
      0x00, 0x02, 0x00, 0x81,
    ST77XX_NORON,     ST_CMD_DELAY,
      10,
    ST77XX_DISPON,    ST_CMD_DELAY,
      255 },

  Rcmd1[] = {
    15,
    ST77XX_SWRESET,   ST_CMD_DELAY,
      150,
    ST77XX_SLPOUT,    ST_CMD_DELAY,
      255,
    ST7735_FRMCTR1, 3,
      0x01, 0x2C, 0x2D,
    ST7735_FRMCTR2, 3,
      0x01, 0x2C, 0x2D,
    ST7735_FRMCTR3, 6,
      0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
    ST7735_INVCTR,  1,
      0x07,
    ST7735_PWCTR1,  3,
      0xA2, 0x02, 0x84,
    ST7735_PWCTR2,  1,
      0xC5,
    ST7735_PWCTR3,  2,
      0x0A, 0x00,
    ST7735_PWCTR4,  2,
      0x8A, 0x2A,
    ST7735_PWCTR5,  2,
      0x8A, 0xEE,
    ST7735_VMCTR1,  1,
      0x0E,
    ST77XX_INVOFF,  0,
    ST77XX_MADCTL,  1,
      0xC8,
    ST77XX_COLMOD,  1,
      0x05 },

  Rcmd2green[] = {
    2,
    ST77XX_CASET,   4,
      0x00, 0x02, 0x00, 0x7F+0x02,
    ST77XX_RASET,   4,
      0x00, 0x01, 0x00, 0x9F+0x01 },

  Rcmd2red[] = {
    2,
    ST77XX_CASET,   4,
      0x00, 0x00, 0x00, 0x7F,
    ST77XX_RASET,   4,
      0x00, 0x00, 0x00, 0x9F },

  Rcmd2green144[] = {
    2,
    ST77XX_CASET,   4,
      0x00, 0x00, 0x00, 0x7F,
    ST77XX_RASET,   4,
      0x00, 0x00, 0x00, 0x7F },

  Rcmd2green160x80[] = {
    2,
    ST77XX_CASET,   4,
      0x00, 0x00, 0x00, 0x4F,
    ST77XX_RASET,   4,
      0x00, 0x00, 0x00, 0x9F },

  Rcmd2green160x80plugin[] = {
    3,
    ST77XX_INVON,  0,
    ST77XX_CASET,   4,
      0x00, 0x00, 0x00, 0x4F,
    ST77XX_RASET,   4,
      0x00, 0x00, 0x00, 0x9F },

  Rcmd3[] = {
    4,
    ST7735_GMCTRP1, 16,
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16,
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON,     ST_CMD_DELAY,
      10,
    ST77XX_DISPON,    ST_CMD_DELAY,
      100 };
// clang-format on

void Adafruit_ST7735::initB(void) {
  commonInit(Bcmd);
  setRotation(0);
}

void Adafruit_ST7735::initR(uint8_t options) {
  commonInit(Rcmd1);
  if (options == INITR_GREENTAB) {
    displayInit(Rcmd2green);
    _colstart = 2;
    _rowstart = 1;
  } else if ((options == INITR_144GREENTAB) || (options == INITR_HALLOWING)) {
    _height = ST7735_TFTHEIGHT_128;
    _width = ST7735_TFTWIDTH_128;
    displayInit(Rcmd2green144);
    _colstart = 2;
    _rowstart = 3;
  } else if (options == INITR_MINI160x80) {
    _height = ST7735_TFTWIDTH_80;
    _width = ST7735_TFTHEIGHT_160;
    displayInit(Rcmd2green160x80);
    _colstart = 24;
    _rowstart = 0;
  } else if (options == INITR_MINI160x80_PLUGIN) {
    _height = ST7735_TFTWIDTH_80;
    _width = ST7735_TFTHEIGHT_160;
    displayInit(Rcmd2green160x80plugin);
    _colstart = 26;
    _rowstart = 1;
    invertOnCommand = ST77XX_INVOFF;
    invertOffCommand = ST77XX_INVON;
  } else {
    displayInit(Rcmd2red);
  }
  displayInit(Rcmd3);

  if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80)) {
    uint8_t data = 0xC0;
    sendCommand(ST77XX_MADCTL, &data, 1);
  }

  if (options == INITR_HALLOWING) {
    tabcolor = INITR_144GREENTAB;
    setRotation(2);
  } else {
    tabcolor = options;
    setRotation(0);
  }
}

void Adafruit_ST7735::setRotation(uint8_t m) {
  uint8_t madctl = 0;
  rotation = m & 3;

  if ((tabcolor == INITR_144GREENTAB) || (tabcolor == INITR_HALLOWING)) {
    _rowstart = (rotation < 2) ? 3 : 1;
  }

  switch (rotation) {
  case 0:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR;
    }
    if (tabcolor == INITR_144GREENTAB) {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 || tabcolor == INITR_MINI160x80_PLUGIN) {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    } else {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 1:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }
    if (tabcolor == INITR_144GREENTAB) {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 || tabcolor == INITR_MINI160x80_PLUGIN) {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    } else {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  case 2:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_RGB;
    } else {
      madctl = ST7735_MADCTL_BGR;
    }
    if (tabcolor == INITR_144GREENTAB) {
      _height = ST7735_TFTHEIGHT_128;
      _width = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 || tabcolor == INITR_MINI160x80_PLUGIN) {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_80;
    } else {
      _height = ST7735_TFTHEIGHT_160;
      _width = ST7735_TFTWIDTH_128;
    }
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
  case 3:
    if ((tabcolor == INITR_BLACKTAB) || (tabcolor == INITR_MINI160x80)) {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;
    } else {
      madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR;
    }
    if (tabcolor == INITR_144GREENTAB) {
      _width = ST7735_TFTHEIGHT_128;
      _height = ST7735_TFTWIDTH_128;
    } else if (tabcolor == INITR_MINI160x80 || tabcolor == INITR_MINI160x80_PLUGIN) {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_80;
    } else {
      _width = ST7735_TFTHEIGHT_160;
      _height = ST7735_TFTWIDTH_128;
    }
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}
