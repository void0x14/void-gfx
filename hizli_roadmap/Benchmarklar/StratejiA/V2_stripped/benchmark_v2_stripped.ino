// =============================================================
// VARIANT_2: Stripped Adafruit for ATmega2560 only
// Same API, dead code removed, direct SPI burst optimization
// ATmega2560 + ST7735 128x160 benchmark
// =============================================================

#include <SPI.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

// =============================================================
// ST7735 constants & command tables
// =============================================================
#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT 160
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_PWCTR6  0xFC
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1
#define ST7735_DELAY   0x80

#define INITR_GREENTAB 0x00
#define INITR_REDTAB   0x01
#define INITR_BLACKTAB 0x02

#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_RGB 0x00

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ST7735_BLACK   0x0000
#define ST7735_WHITE   0xFFFF
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_BLUE    0x001F
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0

// R-type init sequences
static const uint8_t PROGMEM Rcmd1[] = {
    15,
    ST7735_SWRESET, ST7735_DELAY, 150,
    ST7735_SLPOUT,  ST7735_DELAY, 255,
    ST7735_FRMCTR1, 3, 0x01, 0x2C, 0x2D,
    ST7735_FRMCTR2, 3, 0x01, 0x2C, 0x2D,
    ST7735_FRMCTR3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
    ST7735_INVCTR,  1, 0x07,
    ST7735_PWCTR1,  3, 0xA2, 0x02, 0x84,
    ST7735_PWCTR2,  1, 0xC5,
    ST7735_PWCTR3,  2, 0x0A, 0x00,
    ST7735_PWCTR4,  2, 0x8A, 0x2A,
    ST7735_PWCTR5,  2, 0x8A, 0xEE,
    ST7735_VMCTR1,  1, 0x0E,
    ST7735_INVOFF,  0,
    ST7735_COLMOD,  1, 0x05,
    ST7735_CASET,   4, 0x00, 0x02, 0x00, 0x7F,
    ST7735_RASET,   4, 0x00, 0x01, 0x00, 0x9F
};

static const uint8_t PROGMEM Rcmd2green[] = {
    2,
    ST7735_GMCTRP1, 16, 0x02,0x1C,0x07,0x12,0x37,0x32,0x29,0x2D,
                        0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10,
    ST7735_GMCTRN1, 16, 0x03,0x1D,0x07,0x06,0x2E,0x2C,0x29,0x2D,
                        0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10
};

static const uint8_t PROGMEM Rcmd2red[] = {
    2,
    ST7735_GMCTRP1, 16, 0x02,0x1C,0x07,0x12,0x37,0x32,0x29,0x2D,
                        0x29,0x25,0x2B,0x39,0x00,0x01,0x03,0x10,
    ST7735_GMCTRN1, 16, 0x03,0x1D,0x07,0x06,0x2E,0x2C,0x29,0x2D,
                        0x2E,0x2E,0x37,0x3F,0x00,0x00,0x02,0x10
};

static const uint8_t PROGMEM Rcmd3[] = {
    4,
    ST7735_NORON,  ST7735_DELAY, 10,
    ST7735_MADCTL, 1, ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB,
    ST7735_DISPON, ST7735_DELAY, 100,
    0x00
};

// Standard ASCII 5x7 font
static const uint8_t PROGMEM font[] = {
    0x00,0x00,0x00,0x00,0x00, 0x3E,0x5B,0x4F,0x5B,0x3E,
    0x3E,0x6B,0x4F,0x6B,0x3E, 0x1C,0x3E,0x7C,0x3E,0x1C,
    0x18,0x3C,0x7E,0x3C,0x18, 0x1C,0x57,0x7D,0x57,0x1C,
    0x1C,0x5E,0x7F,0x5E,0x1C, 0x00,0x18,0x3C,0x18,0x00,
    0xFF,0xE7,0xC3,0xE7,0xFF, 0x00,0x18,0x24,0x24,0x18,
    0xFF,0xE7,0xDB,0xDB,0xE7, 0x0C,0x12,0x5C,0x38,0x66,
    0x50,0x6E,0x78,0x6E,0x50, 0x60,0x70,0x3F,0x06,0x04,
    0x60,0x70,0x3F,0x06,0x04, 0x00,0x66,0x99,0x8D,0x76,
    0x3C,0x66,0x5A,0x5A,0x66,0x3C, 0x00,0x7F,0x3E,0x1C,0x08,
    0x08,0x1C,0x3E,0x7F,0x00, 0x24,0x66,0xFF,0x66,0x24,
    0x00,0x10,0x38,0x7F,0x38,0x10, 0x00,0x08,0x04,0x7E,0x04,0x08,
    0x00,0x10,0x20,0x7E,0x20,0x10, 0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x5F,0x00, 0x00,0x00,0x07,0x00,0x07,0x00,
    0x14,0x7F,0x14,0x7F,0x14, 0x24,0x2A,0x7F,0x2A,0x12,
    0x23,0x13,0x08,0x64,0x62, 0x36,0x49,0x55,0x22,0x50,
    0x00,0x05,0x03,0x00,0x00, 0x00,0x1C,0x22,0x41,0x00,
    0x00,0x41,0x22,0x1C,0x00, 0x08,0x2A,0x1C,0x2A,0x08,
    0x08,0x08,0x3E,0x08,0x08, 0x00,0x50,0x30,0x00,0x00,
    0x08,0x08,0x08,0x08,0x08, 0x00,0x60,0x60,0x00,0x00,
    0x20,0x10,0x08,0x04,0x02, 0x3E,0x51,0x49,0x45,0x3E,
    0x00,0x42,0x7F,0x40,0x00, 0x42,0x61,0x51,0x49,0x46,
    0x21,0x41,0x45,0x4B,0x31, 0x18,0x14,0x12,0x7F,0x10,
    0x27,0x45,0x45,0x45,0x39, 0x3C,0x4A,0x49,0x49,0x30,
    0x01,0x71,0x09,0x05,0x03, 0x36,0x49,0x49,0x49,0x36,
    0x06,0x49,0x49,0x29,0x1E, 0x00,0x36,0x36,0x00,0x00,
    0x00,0x56,0x36,0x00,0x00, 0x00,0x08,0x14,0x22,0x41,
    0x14,0x14,0x14,0x14,0x14, 0x41,0x22,0x14,0x08,0x00,
    0x02,0x01,0x51,0x09,0x06, 0x32,0x49,0x79,0x41,0x3E,
    0x7E,0x11,0x11,0x11,0x7E, 0x7F,0x49,0x49,0x49,0x36,
    0x3E,0x41,0x41,0x41,0x22, 0x7F,0x41,0x41,0x22,0x1C,
    0x7F,0x49,0x49,0x49,0x41, 0x7F,0x09,0x09,0x01,0x01,
    0x3E,0x41,0x41,0x51,0x32, 0x7F,0x08,0x08,0x08,0x7F,
    0x00,0x41,0x7F,0x41,0x00, 0x20,0x40,0x41,0x3F,0x01,
    0x7F,0x08,0x14,0x22,0x41, 0x7F,0x40,0x40,0x40,0x40,
    0x7F,0x02,0x04,0x02,0x7F, 0x7F,0x04,0x08,0x10,0x7F,
    0x3E,0x41,0x41,0x41,0x3E, 0x7F,0x09,0x09,0x09,0x06,
    0x3E,0x41,0x51,0x21,0x5E, 0x7F,0x09,0x19,0x29,0x46,
    0x46,0x49,0x49,0x49,0x31, 0x01,0x01,0x7F,0x01,0x01,
    0x3F,0x40,0x40,0x40,0x3F, 0x1F,0x20,0x40,0x20,0x1F,
    0x3F,0x40,0x38,0x40,0x3F, 0x63,0x14,0x08,0x14,0x63,
    0x07,0x08,0x70,0x08,0x07, 0x61,0x51,0x49,0x45,0x43,
    0x00,0x7F,0x41,0x41,0x00, 0x02,0x04,0x08,0x10,0x20,
    0x00,0x41,0x41,0x7F,0x00, 0x04,0x02,0x01,0x02,0x04,
    0x40,0x40,0x40,0x40,0x40, 0x00,0x01,0x02,0x04,0x00,
    0x20,0x54,0x54,0x54,0x78, 0x7F,0x48,0x44,0x44,0x38,
    0x38,0x44,0x44,0x44,0x20, 0x38,0x44,0x44,0x48,0x7F,
    0x38,0x54,0x54,0x54,0x18, 0x08,0x7E,0x09,0x01,0x02,
    0x08,0x14,0x54,0x54,0x3C, 0x7F,0x08,0x04,0x04,0x78,
    0x00,0x44,0x7D,0x40,0x00, 0x20,0x40,0x44,0x3D,0x00,
    0x7F,0x10,0x28,0x44,0x00, 0x00,0x41,0x7F,0x40,0x00,
    0x7C,0x04,0x18,0x04,0x78, 0x7C,0x08,0x04,0x04,0x78,
    0x38,0x44,0x44,0x44,0x38, 0x7C,0x14,0x14,0x14,0x08,
    0x08,0x14,0x14,0x18,0x7C, 0x7C,0x08,0x04,0x04,0x08,
    0x48,0x54,0x54,0x54,0x20, 0x04,0x3F,0x44,0x40,0x20,
    0x3C,0x40,0x40,0x20,0x7C, 0x1C,0x20,0x40,0x20,0x1C,
    0x3C,0x40,0x30,0x40,0x3C, 0x44,0x28,0x10,0x28,0x44,
    0x0C,0x50,0x50,0x50,0x3C, 0x44,0x64,0x54,0x4C,0x44,
    0x00,0x08,0x36,0x41,0x00, 0x00,0x00,0x7F,0x00,0x00,
    0x00,0x41,0x36,0x08,0x00, 0x08,0x04,0x08,0x10,0x08,
    0xFF,0xFF,0xFF,0xFF,0xFF
};

#define swap(a, b) { int16_t t = a; a = b; b = t; }

// =============================================================
// ST7735 driver class (self-contained, optimized)
// =============================================================
class ST7735_Driver {
public:
    ST7735_Driver(int8_t cs, int8_t dc, int8_t rst)
        : _cs(cs), _dc(dc), _rst(rst), _width(ST7735_TFTWIDTH),
          _height(ST7735_TFTHEIGHT), _colstart(0), _rowstart(0),
          cursor_x(0), cursor_y(0), textcolor(WHITE), textbgcolor(WHITE),
          textsize(1), rotation(0), wrap(true) {}

    void initR(uint8_t options = INITR_BLACKTAB);
    void setRotation(uint8_t m);

    // Drawing API
    void fillScreen(uint16_t color) { fillRect(0, 0, _width, _height, color); }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);

    // Text API
    void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
    void setTextColor(uint16_t c) { textcolor = textbgcolor = c; }
    void setTextColor(uint16_t c, uint16_t bg) { textcolor = c; textbgcolor = bg; }
    void setTextSize(uint8_t s) { textsize = (s > 0) ? s : 1; }
    void setTextWrap(bool w) { wrap = w; }

    void print(const char* s) {
        while (*s) {
            if (*s == '\n') { cursor_y += textsize * 6; cursor_x = 0; }
            else if (*s != '\r') {
                drawChar(cursor_x, cursor_y, *s, textcolor, textbgcolor, textsize);
                cursor_x += textsize * 6;
                if (wrap && (cursor_x > (_width - textsize * 6))) {
                    cursor_y += textsize * 6;
                    cursor_x = 0;
                }
            }
            s++;
        }
    }

    int16_t width() { return _width; }
    int16_t height() { return _height; }

    // SPI low-level (public for benchmark test 2)
    void startWrite() { PORTB &= ~(1 << 4); }  // CS=PB4 low (direct register)
    void endWrite()   { PORTB |=  (1 << 4); }  // CS=PB4 high

private:
    void sendCommand(uint8_t cmd);
    void sendCommandArgs(uint8_t cmd, const uint8_t* args, uint8_t count);
    void runCommandList(const uint8_t* addr);
    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void writeColor(uint16_t color, uint32_t len);

    // DC pin direct register (DC=pin9=PH6 on ATmega2560)
    void dcLow()  { PORTH &= ~(1 << 6); }
    void dcHigh() { PORTH |=  (1 << 6); }

    int8_t _cs, _dc, _rst;
    int16_t _width, _height;
    uint8_t _colstart, _rowstart;
    int16_t cursor_x, cursor_y;
    uint16_t textcolor, textbgcolor;
    uint8_t textsize, rotation;
    bool wrap;
};

void ST7735_Driver::sendCommand(uint8_t cmd) {
    dcLow();
    SPDR = cmd; while (!(SPSR & (1 << SPIF)));
    dcHigh();
}

void ST7735_Driver::sendCommandArgs(uint8_t cmd, const uint8_t* args, uint8_t count) {
    sendCommand(cmd);
    while (count--) { SPDR = pgm_read_byte(args++); while (!(SPSR & (1 << SPIF))); }
}

void ST7735_Driver::runCommandList(const uint8_t* addr) {
    uint8_t numCommands = pgm_read_byte(addr++);
    while (numCommands--) {
        uint8_t cmd = pgm_read_byte(addr++);
        uint8_t numArgs = pgm_read_byte(addr++);
        uint8_t ms = numArgs & ST7735_DELAY;
        numArgs &= ~ST7735_DELAY;
        sendCommandArgs(cmd, addr, numArgs);
        addr += numArgs;
        if (ms) { ms = pgm_read_byte(addr++); delay(ms == 255 ? 500 : ms); }
    }
}

void ST7735_Driver::initR(uint8_t options) {
    // CS=PB4 output, DC=PH6 output (direct DDR register)
    DDRB |= (1 << 4);
    DDRH |= (1 << 6);
    PORTB |= (1 << 4);  // CS high
    if (_rst >= 0) {
        DDRB |= (1 << 0);     // RST=PB0 output (assumed, change if needed)
        PORTB |= (1 << 0); delay(10);
        PORTB &= ~(1 << 0); delay(10);
        PORTB |= (1 << 0); delay(10);
    }
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

    runCommandList(Rcmd1);
    if (options == INITR_GREENTAB) {
        runCommandList(Rcmd2green);
        _colstart = 2; _rowstart = 1;
    } else {
        runCommandList(Rcmd2red);
    }
    runCommandList(Rcmd3);
}

void ST7735_Driver::setRotation(uint8_t m) {
    uint8_t madctl = 0;
    rotation = m & 3;
    switch (rotation) {
        case 0: madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB;
                _width = ST7735_TFTWIDTH; _height = ST7735_TFTHEIGHT; break;
        case 1: madctl = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
                _width = ST7735_TFTHEIGHT; _height = ST7735_TFTWIDTH; break;
        case 2: madctl = ST7735_MADCTL_RGB;
                _width = ST7735_TFTWIDTH; _height = ST7735_TFTHEIGHT; break;
        case 3: madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
                _width = ST7735_TFTHEIGHT; _height = ST7735_TFTWIDTH; break;
    }
    startWrite();
    sendCommand(ST7735_MADCTL);
    SPDR = madctl; while (!(SPSR & (1 << SPIF)));
    endWrite();
}

void ST7735_Driver::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t xa = x + _colstart, ya = y + _rowstart;
    sendCommand(ST7735_CASET);
    SPDR = xa >> 8;          while (!(SPSR & (1 << SPIF)));
    SPDR = xa & 0xFF;        while (!(SPSR & (1 << SPIF)));
    SPDR = (xa+w-1) >> 8;    while (!(SPSR & (1 << SPIF)));
    SPDR = (xa+w-1) & 0xFF;  while (!(SPSR & (1 << SPIF)));
    sendCommand(ST7735_RASET);
    SPDR = ya >> 8;          while (!(SPSR & (1 << SPIF)));
    SPDR = ya & 0xFF;        while (!(SPSR & (1 << SPIF)));
    SPDR = (ya+h-1) >> 8;    while (!(SPSR & (1 << SPIF)));
    SPDR = (ya+h-1) & 0xFF;  while (!(SPSR & (1 << SPIF)));
    sendCommand(ST7735_RAMWR);
}

void ST7735_Driver::writeColor(uint16_t color, uint32_t len) {
    uint8_t hi = color >> 8, lo = color;
    while (len--) { SPDR = hi; while (!(SPSR & (1 << SPIF)));
                    SPDR = lo; while (!(SPSR & (1 << SPIF))); }
}

void ST7735_Driver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= _width || y >= _height || w <= 0 || h <= 0) return;
    int16_t x2 = x + w - 1, y2 = y + h - 1;
    if (x2 < 0 || y2 < 0) return;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 >= _width)  x2 = _width  - 1;
    if (y2 >= _height) y2 = _height - 1;
    w = x2 - x + 1; h = y2 - y + 1;
    if (w <= 0 || h <= 0) return;
    startWrite();
    setAddrWindow(x, y, w, h);
    writeColor(color, (uint32_t)w * h);
    endWrite();
}

void ST7735_Driver::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    startWrite();
    setAddrWindow(x, y, 1, 1);
    SPDR = color >> 8;   while (!(SPSR & (1 << SPIF)));
    SPDR = color & 0xFF; while (!(SPSR & (1 << SPIF)));
    endWrite();
}

void ST7735_Driver::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    fillRect(x, y, w, 1, color);
}

void ST7735_Driver::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    fillRect(x, y, 1, h, color);
}

void ST7735_Driver::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) { swap(x0, y0); swap(x1, y1); }
    if (x0 > x1) { swap(x0, x1); swap(y0, y1); }
    int16_t dx = x1 - x0, dy = abs(y1 - y0);
    int16_t err = dx / 2, ystep = (y0 < y1) ? 1 : -1;
    for (; x0 <= x1; x0++) {
        if (steep) drawPixel(y0, x0, color);
        else       drawPixel(x0, y0, color);
        err -= dy;
        if (err < 0) { y0 += ystep; err += dx; }
    }
}

void ST7735_Driver::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        drawPixel(x0+x, y0+y, color); drawPixel(x0-x, y0+y, color);
        drawPixel(x0+x, y0-y, color); drawPixel(x0-x, y0-y, color);
        drawPixel(x0+y, y0+x, color); drawPixel(x0-y, y0+x, color);
        drawPixel(x0+y, y0-x, color); drawPixel(x0-y, y0-x, color);
    }
}

void ST7735_Driver::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    drawFastVLine(x0, y0 - r, 2 * r + 1, color);
    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        drawFastVLine(x0+x, y0-y, 2*y+1, color);
        drawFastVLine(x0-x, y0-y, 2*y+1, color);
        drawFastVLine(x0+y, y0-x, 2*x+1, color);
        drawFastVLine(x0-y, y0-x, 2*x+1, color);
    }
}

void ST7735_Driver::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (x >= _width || y >= _height || (x + 6 * size - 1) < 0 || (y + 8 * size - 1) < 0) return;
    if (c >= 176) c++;
    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = pgm_read_byte(&font[c * 5 + i]);
        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1) {
                if (size == 1) drawPixel(x+i, y+j, color);
                else           fillRect(x+i*size, y+j*size, size, size, color);
            } else if (bg != color) {
                if (size == 1) drawPixel(x+i, y+j, bg);
                else           fillRect(x+i*size, y+j*size, size, size, bg);
            }
        }
    }
    if (bg != color) {
        if (size == 1) drawFastVLine(x+5, y, 8, bg);
        else           fillRect(x+5*size, y, size, 8*size, bg);
    }
}

// =============================================================
// Timer1 infrastructure
// =============================================================
volatile uint32_t _ovf = 0;
ISR(TIMER1_OVF_vect) { _ovf++; }
#define TIMER_START() do { cli(); _ovf=0; TCNT1=0; TCCR1A=0; TCCR1B=(1<<CS11); TIMSK1|=(1<<TOIE1); sei(); } while(0)
#define TIMER_STOP(us) do { cli(); TCCR1B=0; uint32_t _t=((_ovf<<16)|TCNT1); sei(); (us)=_t>>1; } while(0)

#define N 100
uint32_t times[N];

uint32_t isqrt(uint32_t n) {
    if (n == 0) return 0;
    uint32_t x = n, y = (x + 1) >> 1;
    while (y < x) { x = y; y = (x + n / x) >> 1; }
    return x;
}
uint32_t arr_min() { uint32_t m = times[0]; for (uint16_t i = 1; i < N; i++) if (times[i] < m) m = times[i]; return m; }
uint32_t arr_max() { uint32_t m = times[0]; for (uint16_t i = 1; i < N; i++) if (times[i] > m) m = times[i]; return m; }
uint32_t arr_mean() { uint32_t s = 0; for (uint16_t i = 0; i < N; i++) s += times[i]; return s / N; }
uint32_t arr_stddev() {
    uint32_t m = arr_mean(), s = 0;
    for (uint16_t i = 0; i < N; i++) { int32_t d = (int32_t)times[i] - (int32_t)m; s += (uint32_t)(d * d); }
    return isqrt(s / N);
}

uint8_t test_num = 0;
void print_result(const char* name) {
    test_num++;
    Serial.print("["); if (test_num < 10) Serial.print("0");
    Serial.print(test_num); Serial.print("] "); Serial.println(name);
    Serial.print("     min: "); Serial.print(arr_min()); Serial.print(" us   max: "); Serial.print(arr_max()); Serial.println(" us");
    Serial.print("     mean: "); Serial.print(arr_mean()); Serial.print(" us   std: "); Serial.print(arr_stddev()); Serial.println(" us");
    Serial.println();
}

// =============================================================
// Benchmark
// =============================================================
ST7735_Driver tft(TFT_CS, TFT_DC, TFT_RST);

void spi_raw_byte(uint8_t b) { SPDR = b; while (!(SPSR & (1 << SPIF))); }

void t_spi_single_byte()  { tft.startWrite(); spi_raw_byte(0xFF); tft.endWrite(); }
void t_spi_burst_256()    { tft.startWrite(); for (uint16_t i = 0; i < 256; i++) spi_raw_byte(0xAA); tft.endWrite(); }
void t_fill_screen()      { tft.fillScreen(ST7735_RED); }
void t_fill_rect_16x16()  { tft.fillRect(0, 0, 16, 16, ST7735_GREEN); }
void t_draw_pixel()       { tft.drawPixel(64, 80, ST7735_BLUE); }
void t_draw_hline()       { tft.drawFastHLine(0, 80, 128, WHITE); }
void t_draw_vline()       { tft.drawFastVLine(64, 0, 160, WHITE); }
void t_draw_diagonal()    { tft.drawLine(0, 0, 127, 159, ST7735_CYAN); }
void t_draw_circle()      { tft.drawCircle(64, 80, 40, ST7735_YELLOW); }
void t_fill_circle()      { tft.fillCircle(64, 80, 30, ST7735_MAGENTA); }
void t_draw_char()        { tft.drawChar(10, 10, 'A', WHITE, BLACK, 1); }
void t_print_string()     { tft.setCursor(0, 0); tft.print("HELLO128"); }

void bench(void (*fn)(), const char* name) {
    fn();
    for (uint16_t i = 0; i < N; i++) {
        uint32_t t;
        TIMER_START();
        fn();
        TIMER_STOP(t);
        times[i] = t;
    }
    print_result(name);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(0);
    tft.fillScreen(ST7735_BLACK);

    Serial.println("================================");
    Serial.println("BENCHMARK: Stripped Adafruit (ATmega2560)");
    Serial.println("ATmega2560 @ 16MHz | ST7735 128x160");
    Serial.println("================================");
    Serial.println();

    bench(t_spi_single_byte, "spi_single_byte");
    bench(t_spi_burst_256,   "spi_burst_256");
    bench(t_fill_screen,     "fill_screen");
    bench(t_fill_rect_16x16, "fill_rect_16x16");
    bench(t_draw_pixel,      "draw_pixel");
    bench(t_draw_hline,      "draw_hline");
    bench(t_draw_vline,      "draw_vline");
    bench(t_draw_diagonal,   "draw_diagonal");
    bench(t_draw_circle,     "draw_circle");
    bench(t_fill_circle,    "fill_circle");
    bench(t_draw_char,       "draw_char");
    bench(t_print_string,   "print_string");

    Serial.println("================================");
    Serial.println("DONE");
    Serial.println("================================");
    while (1);
}

void loop() {}
