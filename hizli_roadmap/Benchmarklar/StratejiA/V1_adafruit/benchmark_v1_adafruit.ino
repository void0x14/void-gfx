// =============================================================
// VARIANT_1: Adafruit_ST7735 + Adafruit_GFX (ATMEGA2560-specific)
// ATmega2560 + ST7735 128x160 benchmark
// SPI pins hardcoded: CS=PB2, DC=PB1, MOSI=PB3, MISO=PB4, SCK=PB5
// =============================================================

#include "Adafruit_GFX.h"
#include "Adafruit_ST7735.h"
#include <SPI.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

#define TFT_CS_PORT PORTB
#define TFT_CS_BIT  4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ---- Timer1 (16-bit, prescaler 8) extended to 32-bit ----
volatile uint32_t _ovf = 0;
ISR(TIMER1_OVF_vect) { _ovf++; }
#define TIMER_START() do { cli(); _ovf=0; TCNT1=0; TCCR1A=0; TCCR1B=(1<<CS11); TIMSK1|=(1<<TOIE1); sei(); } while(0)
#define TIMER_STOP(us) do { cli(); TCCR1B=0; uint32_t _t=((_ovf<<16)|TCNT1); sei(); (us)=_t>>1; } while(0)

#define N 100

uint32_t times[N];

// ---- Statistics ----
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
    Serial.print("[");
    if (test_num < 10) Serial.print("0");
    Serial.print(test_num);
    Serial.print("] ");
    Serial.println(name);
    Serial.print("     min: ");
    Serial.print(arr_min());
    Serial.print(" us   max: ");
    Serial.print(arr_max());
    Serial.println(" us");
    Serial.print("     mean: ");
    Serial.print(arr_mean());
    Serial.print(" us   std: ");
    Serial.print(arr_stddev());
    Serial.println(" us");
    Serial.println();
}

// ---- SPI raw byte (direct register) ----
void spi_raw_byte(uint8_t b) {
    SPDR = b;
    while (!(SPSR & (1 << SPIF)));
}

// ---- Test functions ----
void t_spi_single_byte() {
    TFT_CS_PORT &= ~(1 << TFT_CS_BIT);
    SPDR = 0xFF;
    while (!(SPSR & (1 << SPIF)));
    TFT_CS_PORT |= (1 << TFT_CS_BIT);
}

void t_spi_burst_256() {
    tft.startWrite();
    for (uint16_t i = 0; i < 256; i++) {
        SPDR = 0xAA;
        while (!(SPSR & (1 << SPIF)));
    }
    tft.endWrite();
}

void t_fill_screen() {
    tft.fillScreen(ST7735_RED);
}

void t_fill_rect_16x16() {
    tft.fillRect(0, 0, 16, 16, ST7735_GREEN);
}

void t_draw_pixel() {
    tft.drawPixel(64, 80, ST7735_BLUE);
}

void t_draw_hline() {
    tft.drawFastHLine(0, 80, 128, ST7735_WHITE);
}

void t_draw_vline() {
    tft.drawFastVLine(64, 0, 160, ST7735_WHITE);
}

void t_draw_diagonal() {
    tft.drawLine(0, 0, 127, 159, ST7735_CYAN);
}

void t_draw_circle() {
    tft.drawCircle(64, 80, 40, ST7735_YELLOW);
}

void t_fill_circle() {
    tft.fillCircle(64, 80, 30, ST7735_MAGENTA);
}

void t_draw_char() {
    tft.drawChar(10, 10, 'A', ST7735_WHITE, ST7735_BLACK, 1);
}

void t_print_string() {
    tft.setCursor(0, 0);
    tft.print("HELLO128");
}

// ---- Benchmark runner ----
void bench(void (*fn)(), const char* name) {
    fn(); // warmup (discarded)
    for (uint16_t i = 0; i < N; i++) {
        uint32_t t;
        TIMER_START();
        fn();
        TIMER_STOP(t);
        times[i] = t;
    }
    print_result(name);
}

// ---- Main ----
void setup() {
    Serial.begin(115200);
    while (!Serial);

    tft.initR(INITR_BLACKTAB);
    tft.setRotation(0);
    tft.fillScreen(ST7735_BLACK);

    Serial.println("================================");
    Serial.println("BENCHMARK: Adafruit_GFX+ST7735");
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
