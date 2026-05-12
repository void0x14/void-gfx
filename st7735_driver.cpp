#include "st7735_driver.h"
#include <cstdint>

void ekranBaslat(const uint8_t *addr) {

    uint8_t komutSayisi, komut, argumanSayisi;
    uint16_t ms; // milisaniye

    komutSayisi = pgm_read_byte(addr++); // Toplam komut sayısını oku


}
