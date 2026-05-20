// =============================================================
// VARIANT_4: Bare-metal pure C, NO Arduino framework
// Raw avr-libc only, custom UART via UBRR0H/UBRR0L
// ATmega2560 + ST7735 128x160 benchmark
// =============================================================

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define BAUD 115200
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

// ---- ATmega2560 port mapping ----
// SPI: PB1(SCK), PB2(MOSI), PB3(MISO), PB0(SS)
// Pin 10 = PB4 (CS), Pin 9 = PH6 (DC), Pin 8 = PH5 (RST)

#define CS_DDR   DDRB
#define CS_PORT  PORTB
#define CS_BIT   PB4

#define DC_DDR   DDRH
#define DC_PORT  PORTH
#define DC_BIT   PH6

#define RST_DDR  DDRH
#define RST_PORT PORTH
#define RST_BIT  PH5

#define SPI_DDR  DDRB

#define CS_LOW()  (CS_PORT  &= ~(1 << CS_BIT))
#define CS_HIGH() (CS_PORT  |=  (1 << CS_BIT))
#define DC_CMD()  (DC_PORT  &= ~(1 << DC_BIT))
#define DC_DATA() (DC_PORT  |=  (1 << DC_BIT))
#define RST_LOW() (RST_PORT &= ~(1 << RST_BIT))
#define RST_HIGH()(RST_PORT |=  (1 << RST_BIT))

// =============================================================
// ST7735 constants
// =============================================================
#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT 160
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
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
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST7735_DELAY 0x80

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

// =============================================================
// ASCII 5x7 font
// =============================================================
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

// =============================================================
// Display state
// =============================================================
static int16_t  _width     = ST7735_TFTWIDTH;
static int16_t  _height    = ST7735_TFTHEIGHT;
static uint8_t  _colstart  = 0;
static uint8_t  _rowstart  = 0;
static int16_t  cursor_x   = 0;
static int16_t  cursor_y   = 0;
static uint16_t textcolor   = WHITE;
static uint16_t textbgcolor = WHITE;
static uint8_t  textsize    = 1;

// =============================================================
// Timer1 (16-bit, prescaler 8) extended to 32-bit
// =============================================================
volatile uint32_t _ovf = 0;
ISR(TIMER1_OVF_vect) { _ovf++; }

#define TIMER_START() do { \
    cli(); _ovf=0; TCNT1=0; TCCR1A=0; \
    TCCR1B=(1<<CS11); TIMSK1|=(1<<TOIE1); sei(); \
} while(0)

#define TIMER_STOP(us) do { \
    cli(); TCCR1B=0; \
    uint32_t _t=((_ovf<<16)|TCNT1); sei(); \
    (us)=_t>>1; \
} while(0)

// =============================================================
// Statistics
// =============================================================
#define N 100
uint32_t times[N];

static uint32_t isqrt(uint32_t n) {
    if (n == 0) return 0;
    uint32_t x = n, y = (x + 1) >> 1;
    while (y < x) { x = y; y = (x + n / x) >> 1; }
    return x;
}

static uint32_t arr_min(void) {
    uint32_t m = times[0];
    for (uint16_t i = 1; i < N; i++) if (times[i] < m) m = times[i];
    return m;
}

static uint32_t arr_max(void) {
    uint32_t m = times[0];
    for (uint16_t i = 1; i < N; i++) if (times[i] > m) m = times[i];
    return m;
}

static uint32_t arr_mean(void) {
    uint32_t s = 0;
    for (uint16_t i = 0; i < N; i++) s += times[i];
    return s / N;
}

static uint32_t arr_stddev(void) {
    uint32_t m = arr_mean(), s = 0;
    for (uint16_t i = 0; i < N; i++) {
        int32_t d = (int32_t)times[i] - (int32_t)m;
        s += (uint32_t)(d * d);
    }
    return isqrt(s / N);
}

// =============================================================
// UART (bare-metal, ATmega2560 USART0)
// =============================================================
static void uart_init(void) {
    // Baud rate: 115200 @ 16MHz -> UBRR = 8
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)(UBRR_VAL);
    UCSR0A = 0;                              // no double speed
    UCSR0B = (1 << TXEN0);                   // enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

static void uart_tx(uint8_t c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

static void uart_print(const char* s) {
    while (*s) uart_tx(*s++);
}

static void uart_print_p(const char* s) {
    char c;
    while ((c = pgm_read_byte(s++))) uart_tx(c);
}

static void uart_print_uint(uint32_t v) {
    char buf[11];
    int8_t i = 0;
    if (v == 0) { uart_tx('0'); return; }
    while (v > 0) { buf[i++] = '0' + (v % 10); v /= 10; }
    while (i > 0) uart_tx(buf[--i]);
}

static void uart_println(void) {
    uart_tx('\r');
    uart_tx('\n');
}

static uint8_t test_num = 0;
static void print_result(const char* name) {
    test_num++;
    uart_tx('[');
    if (test_num < 10) uart_tx('0');
    uart_print_uint(test_num);
    uart_print("] ");
    uart_print(name);
    uart_println();

    uart_print("     min: "); uart_print_uint(arr_min());
    uart_print(" us   max: "); uart_print_uint(arr_max());
    uart_print(" us"); uart_println();

    uart_print("     mean: "); uart_print_uint(arr_mean());
    uart_print(" us   std: "); uart_print_uint(arr_stddev());
    uart_print(" us"); uart_println();
    uart_println();
}

// =============================================================
// SPI primitives (direct register, fosc/2)
// =============================================================
static inline void spi_write(uint8_t b) {
    SPDR = b;
    while (!(SPSR & (1 << SPIF)));
}

static inline void spi_write16(uint16_t w) {
    SPDR = w >> 8;   while (!(SPSR & (1 << SPIF)));
    SPDR = w & 0xFF; while (!(SPSR & (1 << SPIF)));
}

static inline void spi_burst_color(uint16_t color, uint32_t count) {
    uint8_t hi = color >> 8, lo = color;
    while (count--) {
        SPDR = hi; while (!(SPSR & (1 << SPIF)));
        SPDR = lo; while (!(SPSR & (1 << SPIF)));
    }
}

// =============================================================
// ST7735 command/data
// =============================================================
static void st_cmd(uint8_t cmd) {
    DC_CMD();
    spi_write(cmd);
    DC_DATA();
}

static void st_cmd_args(uint8_t cmd, const uint8_t* args, uint8_t count) {
    DC_CMD();
    spi_write(cmd);
    DC_DATA();
    while (count--) spi_write(pgm_read_byte(args++));
}

static void st_run_list(const uint8_t* addr) {
    uint8_t ncmd = pgm_read_byte(addr++);
    while (ncmd--) {
        uint8_t cmd = pgm_read_byte(addr++);
        uint8_t nargs = pgm_read_byte(addr++);
        uint8_t ms = nargs & ST7735_DELAY;
        nargs &= ~ST7735_DELAY;
        st_cmd_args(cmd, addr, nargs);
        addr += nargs;
        if (ms) {
            uint8_t d = pgm_read_byte(addr++);
            if (d == 255) d = 200; // cap for _delay_ms compile-time
            // split into 100ms chunks (max compile-time _delay_ms varies)
            while (d > 100) { _delay_ms(100); d -= 100; }
            if (d > 0) _delay_ms(d);
        }
    }
}

// Init sequences for R-type
static const uint8_t PROGMEM Rcmd1[] = {
    15,
    ST7735_SWRESET, ST7735_DELAY, 150,
    ST7735_SLPOUT,  ST7735_DELAY, 200,  // was 255, capped to 200
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

// =============================================================
// Display init & drawing
// =============================================================
static void st_init(void) {
    // GPIO
    CS_DDR  |=  (1 << CS_BIT);
    DC_DDR  |=  (1 << DC_BIT);
    RST_DDR |=  (1 << RST_BIT);
    CS_HIGH();

    // SPI: master, mode 0, fosc/2
    SPI_DDR |= (1 << PB1) | (1 << PB2); // SCK, MOSI output
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR = (1 << SPI2X);

    // Hardware reset
    RST_HIGH(); _delay_ms(10);
    RST_LOW();  _delay_ms(10);
    RST_HIGH(); _delay_ms(10);

    st_run_list(Rcmd1);
    st_run_list(Rcmd2red);
    st_run_list(Rcmd3);
}

static void st_set_rotation(uint8_t m) {
    uint8_t madctl = 0;
    m &= 3;
    switch (m) {
        case 0: madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB;
                _width = ST7735_TFTWIDTH; _height = ST7735_TFTHEIGHT; break;
        case 1: madctl = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
                _width = ST7735_TFTHEIGHT; _height = ST7735_TFTWIDTH; break;
        case 2: madctl = ST7735_MADCTL_RGB;
                _width = ST7735_TFTWIDTH; _height = ST7735_TFTHEIGHT; break;
        case 3: madctl = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB;
                _width = ST7735_TFTHEIGHT; _height = ST7735_TFTWIDTH; break;
    }
    CS_LOW();
    st_cmd(ST7735_MADCTL);
    spi_write(madctl);
    CS_HIGH();
}

static void st_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t xa = x + _colstart, ya = y + _rowstart;
    st_cmd(ST7735_CASET);
    spi_write16(xa); spi_write16(xa + w - 1);
    st_cmd(ST7735_RASET);
    spi_write16(ya); spi_write16(ya + h - 1);
    st_cmd(ST7735_RAMWR);
}

static void st_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= _width || y >= _height || w <= 0 || h <= 0) return;
    int16_t x2 = x + w - 1, y2 = y + h - 1;
    if (x2 < 0 || y2 < 0) return;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 >= _width)  x2 = _width  - 1;
    if (y2 >= _height) y2 = _height - 1;
    w = x2 - x + 1; h = y2 - y + 1;
    if (w <= 0 || h <= 0) return;
    CS_LOW();
    st_set_window(x, y, w, h);
    spi_burst_color(color, (uint32_t)w * h);
    CS_HIGH();
}

static void st_fill_screen(uint16_t color) {
    st_fill_rect(0, 0, _width, _height, color);
}

static void st_draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    CS_LOW();
    st_set_window(x, y, 1, 1);
    spi_write16(color);
    CS_HIGH();
}

static void st_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color) {
    st_fill_rect(x, y, w, 1, color);
}

static void st_draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color) {
    st_fill_rect(x, y, 1, h, color);
}

static void st_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) { int16_t t = x0; x0 = y0; y0 = t; t = x1; x1 = y1; y1 = t; }
    if (x0 > x1) { int16_t t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    int16_t dx = x1 - x0, dy = abs(y1 - y0);
    int16_t err = dx / 2, ystep = (y0 < y1) ? 1 : -1;
    for (; x0 <= x1; x0++) {
        if (steep) st_draw_pixel(y0, x0, color);
        else       st_draw_pixel(x0, y0, color);
        err -= dy;
        if (err < 0) { y0 += ystep; err += dx; }
    }
}

static void st_draw_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    st_draw_pixel(x0, y0+r, color); st_draw_pixel(x0, y0-r, color);
    st_draw_pixel(x0+r, y0, color); st_draw_pixel(x0-r, y0, color);
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        st_draw_pixel(x0+x,y0+y,color); st_draw_pixel(x0-x,y0+y,color);
        st_draw_pixel(x0+x,y0-y,color); st_draw_pixel(x0-x,y0-y,color);
        st_draw_pixel(x0+y,y0+x,color); st_draw_pixel(x0-y,y0+x,color);
        st_draw_pixel(x0+y,y0-x,color); st_draw_pixel(x0-y,y0-x,color);
    }
}

static void st_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    st_draw_vline(x0, y0-r, 2*r+1, color);
    int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
    while (x < y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        st_draw_vline(x0+x, y0-y, 2*y+1, color);
        st_draw_vline(x0-x, y0-y, 2*y+1, color);
        st_draw_vline(x0+y, y0-x, 2*x+1, color);
        st_draw_vline(x0-y, y0-x, 2*x+1, color);
    }
}

static void st_draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t sz) {
    if (x >= _width || y >= _height || (x + 6*sz - 1) < 0 || (y + 8*sz - 1) < 0) return;
    if (c >= 176) c++;
    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = pgm_read_byte(&font[c * 5 + i]);
        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1) {
                if (sz == 1) st_draw_pixel(x+i, y+j, color);
                else         st_fill_rect(x+i*sz, y+j*sz, sz, sz, color);
            } else if (bg != color) {
                if (sz == 1) st_draw_pixel(x+i, y+j, bg);
                else         st_fill_rect(x+i*sz, y+j*sz, sz, sz, bg);
            }
        }
    }
    if (bg != color) {
        if (sz == 1) st_draw_vline(x+5, y, 8, bg);
        else         st_fill_rect(x+5*sz, y, sz, 8*sz, bg);
    }
}

static void st_print(const char* s) {
    while (*s) {
        if (*s == '\n') { cursor_y += textsize * 6; cursor_x = 0; }
        else if (*s != '\r') {
            st_draw_char(cursor_x, cursor_y, *s, textcolor, textbgcolor, textsize);
            cursor_x += textsize * 6;
            if (cursor_x > (_width - textsize * 6)) {
                cursor_y += textsize * 6;
                cursor_x = 0;
            }
        }
        s++;
    }
}

// =============================================================
// Benchmark test functions
// =============================================================
static void t_spi_single_byte(void) {
    CS_LOW();
    spi_write(0xFF);
    CS_HIGH();
}

static void t_spi_burst_256(void) {
    CS_LOW();
    for (uint16_t i = 0; i < 256; i++) spi_write(0xAA);
    CS_HIGH();
}

static void t_fill_screen(void)     { st_fill_screen(RED); }
static void t_fill_rect_16x16(void) { st_fill_rect(0, 0, 16, 16, GREEN); }
static void t_draw_pixel(void)      { st_draw_pixel(64, 80, BLUE); }
static void t_draw_hline(void)      { st_draw_hline(0, 80, 128, WHITE); }
static void t_draw_vline(void)      { st_draw_vline(64, 0, 160, WHITE); }
static void t_draw_diagonal(void)   { st_draw_line(0, 0, 127, 159, CYAN); }
static void t_draw_circle(void)     { st_draw_circle(64, 80, 40, YELLOW); }
static void t_fill_circle(void)     { st_fill_circle(64, 80, 30, MAGENTA); }
static void t_draw_char(void)       { st_draw_char(10, 10, 'A', WHITE, BLACK, 1); }
static void t_print_string(void) {
    cursor_x = 0; cursor_y = 0;
    textcolor = WHITE; textbgcolor = WHITE; textsize = 1;
    st_print("HELLO128");
}

typedef void (*test_fn)(void);

static void bench(test_fn fn, const char* name) {
    fn(); // warmup
    for (uint16_t i = 0; i < N; i++) {
        uint32_t t;
        TIMER_START();
        fn();
        TIMER_STOP(t);
        times[i] = t;
    }
    print_result(name);
}

// =============================================================
// main() — no Arduino, no setup()/loop()
// =============================================================
int main(void) {
    // Disable all Arduino-interrupt leftovers (clean slate)
    cli();
    TIMSK0 = 0;   // disable Timer0 interrupts (we don't touch TCCR0)
    sei();

    uart_init();
    st_init();
    st_set_rotation(0);
    st_fill_screen(BLACK);

    uart_print("================================"); uart_println();
    uart_print("BENCHMARK: Bare-metal avr-libc"); uart_println();
    uart_print("ATmega2560 @ 16MHz | ST7735 128x160"); uart_println();
    uart_print("================================"); uart_println();
    uart_println();

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

    uart_print("================================"); uart_println();
    uart_print("DONE"); uart_println();
    uart_print("================================"); uart_println();

    while (1);
}
