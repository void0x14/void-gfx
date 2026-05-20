/*!
 * @file Adafruit_SPITFT.h
 *
 * ATMEGA2560-specific version. Hardcoded SPI pins on PORTB.
 * CS=PB2, DC=PB1, MOSI=PB3, MISO=PB4, SCK=PB5
 * Software SPI, parallel interface, DMA removed.
 */

#ifndef _ADAFRUIT_SPITFT_H_
#define _ADAFRUIT_SPITFT_H_

#include "Adafruit_GFX.h"

// ATMEGA2560 SPI pin definitions (PORTB)
#define SPI_DDR   DDRB
#define SPI_PORT  PORTB
#define SPI_PIN   PINB

#define CS_BIT    2    // PB2
#define DC_BIT    1    // PB1
#define MOSI_BIT  3    // PB3
#define MISO_BIT  4    // PB4
#define SCK_BIT   5    // PB5
#define SS_BIT    0    // PB0

#define CS_MASK   (1 << CS_BIT)
#define DC_MASK   (1 << DC_BIT)
#define MOSI_MASK (1 << MOSI_BIT)
#define MISO_MASK (1 << MISO_BIT)
#define SCK_MASK  (1 << SCK_BIT)

class Adafruit_SPITFT : public Adafruit_GFX {

public:
  Adafruit_SPITFT(uint16_t w, uint16_t h, int8_t rst = -1);
  ~Adafruit_SPITFT(){};

  virtual void begin(uint32_t freq) = 0;
  virtual void setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                             uint16_t h) = 0;

  void initSPI(void);
  void startWrite(void);
  void endWrite(void);
  void sendCommand(uint8_t commandByte, uint8_t *dataBytes,
                   uint8_t numDataBytes);
  void sendCommand(uint8_t commandByte, const uint8_t *dataBytes = NULL,
                   uint8_t numDataBytes = 0);
  uint8_t readcommand8(uint8_t commandByte, uint8_t index = 0);

  void writePixel(int16_t x, int16_t y, uint16_t color);
  void writePixels(uint16_t *colors, uint32_t len);
  void writeColor(uint16_t color, uint32_t len);
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color);
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  inline void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w,
                                       int16_t h, uint16_t color);

  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void pushColor(uint16_t color);

  using Adafruit_GFX::drawRGBBitmap;
  void drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors, int16_t w,
                     int16_t h);

  void invertDisplay(bool i);
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

  void spiWrite(uint8_t b);
  void writeCommand(uint8_t cmd);
  uint8_t spiRead(void);

  void SPI_WRITE16(uint16_t w);
  void SPI_WRITE32(uint32_t l);

  void SPI_CS_HIGH(void) { SPI_PORT |= CS_MASK; }
  void SPI_CS_LOW(void)  { SPI_PORT &= ~CS_MASK; }
  void SPI_DC_HIGH(void) { SPI_PORT |= DC_MASK; }
  void SPI_DC_LOW(void)  { SPI_PORT &= ~DC_MASK; }

protected:
  int8_t _rst;
  int16_t _xstart = 0;
  int16_t _ystart = 0;
  uint8_t invertOnCommand = 0;
  uint8_t invertOffCommand = 0;
};

#endif
