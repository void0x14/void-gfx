/*!
 * @file Adafruit_SPITFT.cpp
 *
 * ATMEGA2560-specific version. Hardcoded SPI on PORTB.
 * CS=PB2, DC=PB1, MOSI=PB3, MISO=PB4, SCK=PB5
 * Direct register access. No Arduino helpers. No soft SPI. No parallel.
 */

#include "Adafruit_SPITFT.h"

// --- ATMEGA2560 SPI Macros ---
#define SPI_WRITE_BYTE(x) for (SPDR = (x); !(SPSR & _BV(SPIF));)
#define SPI_READ_BYTE()   (SPDR)

// Constructor
Adafruit_SPITFT::Adafruit_SPITFT(uint16_t w, uint16_t h, int8_t rst)
    : Adafruit_GFX(w, h), _rst(rst) {}

// --- Hardware init ---
void Adafruit_SPITFT::initSPI(void) {
  // Set MOSI, SCK, SS, DC as output; MISO as input
  SPI_DDR |= MOSI_MASK | SCK_MASK | SS_MASK | CS_MASK | DC_MASK;
  SPI_DDR &= ~MISO_MASK;

  // Deselect CS, set DC high (data mode)
  SPI_PORT |= CS_MASK | DC_MASK;

  // SPI Enable, Master, MSB first, Mode 0, Fosc/2 (SPI2X)
  SPCR = _BV(SPE) | _BV(MSTR);
  SPSR = _BV(SPI2X);

  // Reset sequence
  if (_rst >= 0) {
    DDRB |= _BV(0);   // RST pin as output (assume PB0 or change)
    PORTB |= _BV(0);
    delay(100);
    PORTB &= ~_BV(0);
    delay(100);
    PORTB |= _BV(0);
    delay(200);
  }
}

// --- Transaction control ---
void Adafruit_SPITFT::startWrite(void) {
  SPI_PORT &= ~CS_MASK;  // CS LOW
}

void Adafruit_SPITFT::endWrite(void) {
  SPI_PORT |= CS_MASK;   // CS HIGH
}

// --- Low-level SPI ---
void Adafruit_SPITFT::spiWrite(uint8_t b) {
  SPI_WRITE_BYTE(b);
}

uint8_t Adafruit_SPITFT::spiRead(void) {
  SPI_WRITE_BYTE(0);
  return SPI_READ_BYTE();
}

void Adafruit_SPITFT::writeCommand(uint8_t cmd) {
  SPI_PORT &= ~DC_MASK;   // DC LOW (command)
  SPI_WRITE_BYTE(cmd);
  SPI_PORT |= DC_MASK;    // DC HIGH (data)
}

void Adafruit_SPITFT::SPI_WRITE16(uint16_t w) {
  SPI_WRITE_BYTE(w >> 8);
  SPI_WRITE_BYTE(w);
}

void Adafruit_SPITFT::SPI_WRITE32(uint32_t l) {
  SPI_WRITE_BYTE(l >> 24);
  SPI_WRITE_BYTE(l >> 16);
  SPI_WRITE_BYTE(l >> 8);
  SPI_WRITE_BYTE(l);
}

// --- Send command with data ---
void Adafruit_SPITFT::sendCommand(uint8_t commandByte, uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  SPI_PORT &= ~CS_MASK;   // CS LOW

  SPI_PORT &= ~DC_MASK;   // DC LOW
  SPI_WRITE_BYTE(commandByte);
  SPI_PORT |= DC_MASK;    // DC HIGH

  for (uint8_t i = 0; i < numDataBytes; i++) {
    SPI_WRITE_BYTE(*dataBytes++);
  }

  SPI_PORT |= CS_MASK;    // CS HIGH
}

void Adafruit_SPITFT::sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  SPI_PORT &= ~CS_MASK;

  SPI_PORT &= ~DC_MASK;
  SPI_WRITE_BYTE(commandByte);
  SPI_PORT |= DC_MASK;

  for (uint8_t i = 0; i < numDataBytes; i++) {
    SPI_WRITE_BYTE(pgm_read_byte(dataBytes++));
  }

  SPI_PORT |= CS_MASK;
}

uint8_t Adafruit_SPITFT::readcommand8(uint8_t commandByte, uint8_t index) {
  uint8_t result;
  startWrite();
  SPI_PORT &= ~DC_MASK;
  SPI_WRITE_BYTE(commandByte);
  SPI_PORT |= DC_MASK;
  do {
    result = spiRead();
  } while (index--);
  endWrite();
  return result;
}

// --- Pixel writing ---
void Adafruit_SPITFT::writePixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
  }
}

void Adafruit_SPITFT::writePixels(uint16_t *colors, uint32_t len) {
  while (len--) {
    SPI_WRITE16(*colors++);
  }
}

void Adafruit_SPITFT::writeColor(uint16_t color, uint32_t len) {
  if (!len) return;
  uint8_t hi = color >> 8, lo = color;
  while (len--) {
    SPI_WRITE_BYTE(hi);
    SPI_WRITE_BYTE(lo);
  }
}

// --- Area fills ---
void Adafruit_SPITFT::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                    uint16_t color) {
  if (w && h) {
    if (w < 0) { x += w + 1; w = -w; }
    if (x < _width) {
      if (h < 0) { y += h + 1; h = -h; }
      if (y < _height) {
        int16_t x2 = x + w - 1;
        if (x2 >= 0) {
          int16_t y2 = y + h - 1;
          if (y2 >= 0) {
            if (x < 0) { x = 0; w = x2 + 1; }
            if (y < 0) { y = 0; h = y2 + 1; }
            if (x2 >= _width)  { w = _width - x; }
            if (y2 >= _height) { h = _height - y; }
            writeFillRectPreclipped(x, y, w, h, color);
          }
        }
      }
    }
  }
}

void Adafruit_SPITFT::writeFastHLine(int16_t x, int16_t y, int16_t w,
                                     uint16_t color) {
  if ((y >= 0) && (y < _height) && w) {
    if (w < 0) { x += w + 1; w = -w; }
    if (x < _width) {
      int16_t x2 = x + w - 1;
      if (x2 >= 0) {
        if (x < 0) { x = 0; w = x2 + 1; }
        if (x2 >= _width) { w = _width - x; }
        writeFillRectPreclipped(x, y, w, 1, color);
      }
    }
  }
}

void Adafruit_SPITFT::writeFastVLine(int16_t x, int16_t y, int16_t h,
                                     uint16_t color) {
  if ((x >= 0) && (x < _width) && h) {
    if (h < 0) { y += h + 1; h = -h; }
    if (y < _height) {
      int16_t y2 = y + h - 1;
      if (y2 >= 0) {
        if (y < 0) { y = 0; h = y2 + 1; }
        if (y2 >= _height) { h = _height - y; }
        writeFillRectPreclipped(x, y, 1, h, color);
      }
    }
  }
}

inline void Adafruit_SPITFT::writeFillRectPreclipped(int16_t x, int16_t y,
                                                     int16_t w, int16_t h,
                                                     uint16_t color) {
  setAddrWindow(x, y, w, h);
  writeColor(color, (uint32_t)w * h);
}

// --- Self-contained drawing (with CS/transaction) ---
void Adafruit_SPITFT::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    startWrite();
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
    endWrite();
  }
}

void Adafruit_SPITFT::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  if (w && h) {
    if (w < 0) { x += w + 1; w = -w; }
    if (x < _width) {
      if (h < 0) { y += h + 1; h = -h; }
      if (y < _height) {
        int16_t x2 = x + w - 1;
        if (x2 >= 0) {
          int16_t y2 = y + h - 1;
          if (y2 >= 0) {
            if (x < 0) { x = 0; w = x2 + 1; }
            if (y < 0) { y = 0; h = y2 + 1; }
            if (x2 >= _width)  { w = _width - x; }
            if (y2 >= _height) { h = _height - y; }
            startWrite();
            writeFillRectPreclipped(x, y, w, h, color);
            endWrite();
          }
        }
      }
    }
  }
}

void Adafruit_SPITFT::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                    uint16_t color) {
  if ((y >= 0) && (y < _height) && w) {
    if (w < 0) { x += w + 1; w = -w; }
    if (x < _width) {
      int16_t x2 = x + w - 1;
      if (x2 >= 0) {
        if (x < 0) { x = 0; w = x2 + 1; }
        if (x2 >= _width) { w = _width - x; }
        startWrite();
        writeFillRectPreclipped(x, y, w, 1, color);
        endWrite();
      }
    }
  }
}

void Adafruit_SPITFT::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                    uint16_t color) {
  if ((x >= 0) && (x < _width) && h) {
    if (h < 0) { y += h + 1; h = -h; }
    if (y < _height) {
      int16_t y2 = y + h - 1;
      if (y2 >= 0) {
        if (y < 0) { y = 0; h = y2 + 1; }
        if (y2 >= _height) { h = _height - y; }
        startWrite();
        writeFillRectPreclipped(x, y, 1, h, color);
        endWrite();
      }
    }
  }
}

void Adafruit_SPITFT::pushColor(uint16_t color) {
  startWrite();
  SPI_WRITE16(color);
  endWrite();
}

void Adafruit_SPITFT::drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors,
                                    int16_t w, int16_t h) {
  int16_t x2, y2;
  if ((x >= _width) || (y >= _height) || ((x2 = (x + w - 1)) < 0) ||
      ((y2 = (y + h - 1)) < 0))
    return;

  int16_t bx1 = 0, by1 = 0, saveW = w;
  if (x < 0) { w += x; bx1 = -x; x = 0; }
  if (y < 0) { h += y; by1 = -y; y = 0; }
  if (x2 >= _width)  { w = _width - x; }
  if (y2 >= _height) { h = _height - y; }

  pcolors += by1 * saveW + bx1;
  startWrite();
  setAddrWindow(x, y, w, h);
  while (h--) {
    writePixels(pcolors, w);
    pcolors += saveW;
  }
  endWrite();
}

void Adafruit_SPITFT::invertDisplay(bool i) {
  startWrite();
  writeCommand(i ? invertOnCommand : invertOffCommand);
  endWrite();
}

uint16_t Adafruit_SPITFT::color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}
