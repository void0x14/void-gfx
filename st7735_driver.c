#include "st7735_driver.h"
#include "drivers/st7735_driver.h"
#include <stdint.h>

void ekranBaslat(const uint8_t *addr) {

    uint8_t komutSayisi, komut, argumanSayisi, gercekArgSayisi;
    uint16_t ms; // milisaniye

    komutSayisi = pgm_read_byte(addr++); // Toplam komut sayısını oku

    // Her bir komut için döngüye gir

    while (komutSayisi--) {
        komut = pgm_read_byte(addr++); // Komutu oku
        argumanSayisi = pgm_read_byte(addr++); // Arguman sayısını ve flag'i oku

        gercekArgSayisi = argumanSayisi & ST7735_ARG_MASK; // Maskeleme (Bitwise) yaparak gerçek arguman sayısını alıyoruz

        if (argumanSayisi & ST7735_DELAY_FLAG) {
            ms = pgm_read_byte(addr++);
        }
        else continue;

        }


}
