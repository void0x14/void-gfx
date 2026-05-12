/*!
 * @file Adafruit_SPITFT.h
 *
 * Part of Adafruit's GFX graphics library. Originally this class was
 * written to handle a range of color TFT displays connected via SPI,
 * but over time this library and some display-specific subclasses have
 * mutated to include some color OLEDs as well as parallel-interfaced
 * displays. The name's been kept for the sake of older code.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor "ladyada" Fried for Adafruit Industries,
 * with contributions from the open source community.
 *
 * BSD license, all text here must be included in any redistribution.
 */

#ifndef _ADAFRUIT_SPITFT_H_
#define _ADAFRUIT_SPITFT_H_

#if !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny84__)

#include "Adafruit_GFX.h"
#include <SPI.h>

typedef uint8_t ADAGFX_PORT_t;
#define USE_FAST_PINIO

typedef volatile ADAGFX_PORT_t *PORTreg_t;

#define DEFAULT_SPI_FREQ 8000000L

enum tftBusWidth { tft8bitbus, tft16bitbus };

class Adafruit_SPITFT : public Adafruit_GFX {

public:
  Adafruit_SPITFT(uint16_t w, uint16_t h, int8_t cs, int8_t dc, int8_t mosi,
                  int8_t sck, int8_t rst = -1, int8_t miso = -1);

  Adafruit_SPITFT(uint16_t w, uint16_t h, int8_t cs, int8_t dc,
                  int8_t rst = -1);

  Adafruit_SPITFT(uint16_t w, uint16_t h, SPIClass *spiClass, int8_t cs,
                  int8_t dc, int8_t rst = -1);

  Adafruit_SPITFT(uint16_t w, uint16_t h, tftBusWidth busWidth, int8_t d0,
                  int8_t wr, int8_t dc, int8_t cs = -1, int8_t rst = -1,
                  int8_t rd = -1);

  ~Adafruit_SPITFT(){};

  virtual void begin(uint32_t freq) = 0;
  virtual void setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                             uint16_t h) = 0;

  void initSPI(uint32_t freq = 0, uint8_t spiMode = SPI_MODE0);
  void setSPISpeed(uint32_t freq);
  void startWrite(void);
  void endWrite(void);
  void sendCommand(uint8_t commandByte, uint8_t *dataBytes,
                   uint8_t numDataBytes);
  void sendCommand(uint8_t commandByte, const uint8_t *dataBytes = NULL,
                   uint8_t numDataBytes = 0);
  void sendCommand16(uint16_t commandWord, const uint8_t *dataBytes = NULL,
                     uint8_t numDataBytes = 0);
  uint8_t readcommand8(uint8_t commandByte, uint8_t index = 0);
  uint16_t readcommand16(uint16_t addr);

  void writePixel(int16_t x, int16_t y, uint16_t color);
  void writePixels(uint16_t *colors, uint32_t len, bool block = true,
                   bool bigEndian = false);
  void writeColor(uint16_t color, uint32_t len);
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color);
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  inline void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w,
                                       int16_t h, uint16_t color);
  void dmaWait(void);
  bool dmaBusy(void) const;
  void swapBytes(uint16_t *src, uint32_t len, uint16_t *dest = NULL);

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
  void write16(uint16_t w);
  void writeCommand16(uint16_t cmd);
  uint16_t read16(void);

  void SPI_WRITE16(uint16_t w);
  void SPI_WRITE32(uint32_t l);

  void SPI_CS_HIGH(void) {
    *csPort |= csPinMaskSet;
  }

  void SPI_CS_LOW(void) {
    *csPort &= csPinMaskClr;
  }

  void SPI_DC_HIGH(void) {
    *dcPort |= dcPinMaskSet;
  }

  void SPI_DC_LOW(void) {
    *dcPort &= dcPinMaskClr;
  }

protected:
  inline void SPI_MOSI_HIGH(void);
  inline void SPI_MOSI_LOW(void);
  inline void SPI_SCK_HIGH(void);
  inline void SPI_SCK_LOW(void);
  inline bool SPI_MISO_READ(void);
  inline void SPI_BEGIN_TRANSACTION(void);
  inline void SPI_END_TRANSACTION(void);
  inline void TFT_WR_STROBE(void);
  inline void TFT_RD_HIGH(void);
  inline void TFT_RD_LOW(void);

  PORTreg_t csPort;
  PORTreg_t dcPort;

#if defined(__cplusplus) && (__cplusplus >= 201100)
  union {
#endif
    struct {
      SPIClass *_spi;
#if defined(SPI_HAS_TRANSACTION)
      SPISettings settings;
#else
      uint32_t _freq;
#endif
      uint32_t _mode;
    } hwspi;
    struct {
      PORTreg_t misoPort;
      PORTreg_t mosiPort;
      PORTreg_t sckPort;
      ADAGFX_PORT_t mosiPinMaskSet;
      ADAGFX_PORT_t mosiPinMaskClr;
      ADAGFX_PORT_t sckPinMaskSet;
      ADAGFX_PORT_t sckPinMaskClr;
      ADAGFX_PORT_t misoPinMask;
      int8_t _mosi;
      int8_t _miso;
      int8_t _sck;
    } swspi;
    struct {
      volatile uint8_t *writePort;
      volatile uint8_t *readPort;
      volatile uint8_t *portDir;
      PORTreg_t wrPort;
      PORTreg_t rdPort;
      ADAGFX_PORT_t wrPinMaskSet;
      ADAGFX_PORT_t wrPinMaskClr;
      ADAGFX_PORT_t rdPinMaskSet;
      ADAGFX_PORT_t rdPinMaskClr;
      int8_t _d0;
      int8_t _wr;
      int8_t _rd;
      bool wide = 0;
    } tft8;
#if defined(__cplusplus) && (__cplusplus >= 201100)
  };
#endif

  ADAGFX_PORT_t csPinMaskSet;
  ADAGFX_PORT_t csPinMaskClr;
  ADAGFX_PORT_t dcPinMaskSet;
  ADAGFX_PORT_t dcPinMaskClr;

  uint8_t connection;
  int8_t _rst;
  int8_t _cs;
  int8_t _dc;

  int16_t _xstart = 0;
  int16_t _ystart = 0;
  uint8_t invertOnCommand = 0;
  uint8_t invertOffCommand = 0;

  uint32_t _freq = 0;
};

#endif
#endif
