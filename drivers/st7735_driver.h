#ifndef ST7735_DRIVER_H
#define ST7735_DRIVER_H

#include <stdint.h>
#include <avr/pgmspace.h>

// --- Yazılım Algoritma Bayrakları ---
#define ST7735_DELAY_FLAG 0x80 // Gecikme (delay) olduğunu belirten işaret
#define ST7735_ARG_MASK   0x7F // Sadece gerçek argüman sayısını almak için maske

// --- ST7735 Komut Listesi (Datasheet Bölüm 10) ---
#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09
#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13
#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E
#define ST7735_PTLAR   0x30
#define ST7735_TEOFF   0x34
#define ST7735_TEON    0x35
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A

// --- Panel Ayar Komutları ---
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

// --- MADCTL Bit Tanımları ---
#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_ML  0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08

// --- Başlatma Komut Dizileri ---

static const uint8_t PROGMEM
Bcmd[] = {                         // 7735B ekranlar için başlatma (init) komutları
    18,                             // Listede 18 adet komut var:
    ST7735_SWRESET,   ST7735_DELAY_FLAG, //  1: Software reset (Yazılımsal sıfırlama), argüman yok, gecikmeli
      50,                           //     50 ms gecikme
    ST7735_SLPOUT,    ST7735_DELAY_FLAG, //  2: Sleep mode'dan çıkış (Uyku modunu kapat), argüman yok, gecikmeli
      255,                          //     255 = maksimum (500 ms) gecikme
    ST7735_COLMOD,  1+ST7735_DELAY_FLAG, //  3: Color mode (Renk modu) ayarı, 1 argüman + gecikme:
      0x05,                         //     16-bit renk
      10,                           //     10 ms gecikme
    ST7735_FRMCTR1, 3+ST7735_DELAY_FLAG, //  4: Frame rate (Kare hızı) kontrolü, 3 argüman + gecikme:
      0x00,                         //     En hızlı yenileme (fastest refresh)
      0x06,                         //     6 satır front porch
      0x03,                         //     3 satır back porch
      10,                           //     10 ms gecikme
    ST7735_MADCTL,  1,              //  5: Mem access ctl (Bellek erişim kontrolü - yönler), 1 argüman:
      0x08,                         //     Satır/Sütun adresi, aşağıdan-yukarıya yenileme
    ST7735_DISSET5, 2,              //  6: Ekran ayarları #5, 2 argüman:
      0x15,                         //     1 clk döngüsü nonoverlap, 2 döngü gate
                                    //     rise, 3 döngü osc equalize
      0x02,                         //     VTL üzerine sabitle
    ST7735_INVCTR,  1,              //  7: Display inversion (Ekran tersleme) kontrolü, 1 argüman:
      0x0,                          //     Line inversion (Satır tersleme)
    ST7735_PWCTR1,  2+ST7735_DELAY_FLAG, //  8: Power control (Güç kontrolü), 2 argüman + gecikme:
      0x02,                         //     GVDD = 4.7V
      0x70,                         //     1.0uA
      10,                           //     10 ms gecikme
    ST7735_PWCTR2,  1,              //  9: Power control (Güç kontrolü), 1 argüman, gecikme yok:
      0x05,                         //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3,  2,              // 10: Power control (Güç kontrolü), 2 argüman, gecikme yok:
      0x01,                         //     Opamp akımı düşük (small)
      0x02,                         //     Boost frekansı
    ST7735_VMCTR1,  2+ST7735_DELAY_FLAG, // 11: Power control (Güç kontrolü), 2 argüman + gecikme:
      0x3C,                         //     VCOMH = 4V
      0x38,                         //     VCOML = -1.1V
      10,                           //     10 ms gecikme
    ST7735_PWCTR6,  2,              // 12: Power control (Güç kontrolü), 2 argüman, gecikme yok:
      0x11, 0x15,
    ST7735_GMCTRP1, 16,             // 13: Gamma Ayarlamaları (pozitif polarite), 16 argüman:
      0x09, 0x16, 0x09, 0x20,       //     (Tamamen şart değil ama renklerin
      0x21, 0x1B, 0x13, 0x19,       //      doğru görünmesini sağlar)
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1, 16+ST7735_DELAY_FLAG, // 14: Gamma Ayarlamaları (negatif polarite), 16 argüman + gecikme:
      0x0B, 0x14, 0x08, 0x1E,       //     (Tamamen şart değil ama renklerin
      0x22, 0x1D, 0x18, 0x1E,       //      doğru görünmesini sağlar)
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,                           //     10 ms gecikme
    ST7735_CASET,   4,              // 15: Column addr set (Sütun adresi atama), 4 argüman, gecikme yok:
      0x00, 0x02,                   //     X-BAŞLANGIÇ = 2
      0x00, 0x81,                   //     X-BİTİŞ = 129
    ST7735_RASET,   4,              // 16: Row addr set (Satır adresi atama), 4 argüman, gecikme yok:
      0x00, 0x02,                   //     Y-BAŞLANGIÇ = 1
      0x00, 0x81,                   //     Y-BİTİŞ = 160
    ST7735_NORON,     ST7735_DELAY_FLAG, // 17: Normal display on (Normal ekran modu açık), argüman yok, gecikmeli
      10,                           //     10 ms gecikme
    ST7735_DISPON,    ST7735_DELAY_FLAG, // 18: Main screen turn on (Ana ekranı aç), argüman yok, gecikmeli
      255 },                        //     255 = maksimum (500 ms) gecikme

Rcmd1[] = {                        // 7735R başlatma, Bölüm 1 (Kırmızı veya yeşil tablı ekranlar)
    15,                             // Listede 15 adet komut var:
    ST7735_SWRESET,   ST7735_DELAY_FLAG, //  1: Software reset, 0 argüman, gecikmeli
      150,                          //     150 ms gecikme
    ST7735_SLPOUT,    ST7735_DELAY_FLAG, //  2: Sleep mode'dan çıkış, 0 argüman, gecikmeli
      255,                          //     500 ms gecikme
    ST7735_FRMCTR1, 3,              //  3: Framerate kontrolü - normal mod, 3 argüman:
      0x01, 0x2C, 0x2D,             //     Hız = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate kontrolü - idle (boşta) modu, 3 argüman:
      0x01, 0x2C, 0x2D,             //     Hız = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate kontrolü - partial (kısmi) mod, 6 argüman:
      0x01, 0x2C, 0x2D,             //     Dot inversion modu
      0x01, 0x2C, 0x2D,             //     Line inversion modu
    ST7735_INVCTR,  1,              //  6: Display inversion kontrolü, 1 argüman:
      0x07,                         //     Inversion yok (Tersleme kapalı)
    ST7735_PWCTR1,  3,              //  7: Power control, 3 argüman, gecikme yok:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO modu
    ST7735_PWCTR2,  1,              //  8: Power control, 1 argüman, gecikme yok:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3,  2,              //  9: Power control, 2 argüman, gecikme yok:
      0x0A,                         //     Opamp akımı düşük
      0x00,                         //     Boost frekansı
    ST7735_PWCTR4,  2,              // 10: Power control, 2 argüman, gecikme yok:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp akımı düşük & orta düşük
    ST7735_PWCTR5,  2,              // 11: Power control, 2 argüman, gecikme yok:
      0x8A, 0xEE,
    ST7735_VMCTR1,  1,              // 12: Power control, 1 argüman, gecikme yok:
      0x0E,
    ST7735_INVOFF,  0,              // 13: Ekran terslemeyi kapat (Don't invert), argüman yok
    ST7735_MADCTL,  1,              // 14: Mem access ctl (yönlendirmeler), 1 argüman:
      0xC8,                         //     satır/sütun adresi, aşağıdan-yukarıya yenileme
    ST7735_COLMOD,  1,              // 15: Renk modu ayarı, 1 argüman, gecikme yok:
      0x05 };                       //     16-bit renk

#endif
