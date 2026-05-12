#include "Adafruit_SPITFT.h"

// Mega 2560 (AVR) icin donanimsal SPI yazma makrosu
// Burada ki 'x',makroya disaridan verdigimiz veridir (byte)
// Yani 'x' fonksiyona gonderdigimiz 'parametre'dir.Makronun ismiyle alakasi yok.

// A. Baslangic: SPDR = (x)Bu hamle, mermiyi namluya surmektir.
// $SPDR$ (SPI Data Register): SPI'in veri deposudur.
// Sen bu register'a bir deger atadigin an, Mega 2560'in donanimi otomatik olarak o 8 biti bakir kablodan (MOSI) disari firlatmaya baslar.
// Yani yazma islemi bu esitleme yapildigi an baslar agam.

// B. Kosul: (!(SPSR & _BV(SPIF)))Iste zurnanin "zirt" dedigi, senin "anlayamadin" dedigin bekci burasi.
// $SPSR$ (SPI Status Register): SPI'in durum raporudur.
// $SPIF$ (SPI Interrupt Flag): Bu bir "bayrak"tir.
// Veri gonderme islemi bittigi an donanim bu bayragi 1 yapar.
// _BV(SPIF): Bu bir maskelemedir (Suzgec). "Sadece SPIF bitine bak, digerleri umrumda degil" der.
// SPSR & _BV(SPIF): Eger islem bitmediyse sonuc 0 cikar.
// Islem bittiyse sonuc 1 (veya 0'dan buyuk) cikar.
// Bastaki ! (NOT) Isareti: "Islem bitmedigi surece (0), bu dongunun icinde don dur" demektir.
#define AVR_WRITESPI(x) for (SPDR = (x); (!(SPSR & _BV(SPIF)));)

#define TFT_HARD_SPI 0

void Adafruit_SPITFT::writeColor(uint16_t color, uint32_t len) {
  if (!len)
    return;

  uint8_t hi = color >> 8, lo = color;

  if (connection == TFT_HARD_SPI) {
    while (len--) {
      AVR_WRITESPI(hi);
      AVR_WRITESPI(lo);
    }
  }
}

void Adafruit_SPITFT::spiWrite(uint8_t b) {
  if (connection == TFT_HARD_SPI) {
    AVR_WRITESPI(b);
  }
}

void Adafruit_SPITFT::SPI_WRITE16(uint16_t w) {
  if (connection == TFT_HARD_SPI) {
    AVR_WRITESPI(w >> 8);
    AVR_WRITESPI(w);
  }
}

void Adafruit_SPITFT::SPI_WRITE32(uint32_t l) {
  if (connection == TFT_HARD_SPI) {
    AVR_WRITESPI(l >> 24);
    AVR_WRITESPI(l >> 16);
    AVR_WRITESPI(l >> 8);
    AVR_WRITESPI(l);
  }
}
