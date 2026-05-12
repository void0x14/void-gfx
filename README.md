BAŞLANGIÇ: 5.5.2026
BİTİŞ: X.X.XXXX
-------
KONU: Adafruit GFX yerine kendi minimal gfx'imizi yazmak. (Başta sadece ST7735 için,sonrasında dinamik ağaç ile tak çıkar moduler sıstem sayesınde kendisi donanımı tespit ederek,donanıma özel,donanıma optimize bir şekilde şekil alan proaktif bir kütüphane yazma hedefi)
-------
KONU 2: Adafruit ST7735.h yerine kendi donanım kütüphanemizi yazacağız ve Zero dependency olarak iş yapacağız,bu yuzden SPI.h'a ihtiyaç duymayacağız.Donanımın kendi SPI arayuzunu kullanacagız.
-------
Ana Roadmap [C'ye dönüştürülecek]
-------
1. Adafruit_GFX.cpp oku
drawPixel bul. drawLine bul. drawLine'ın drawPixel'i nasıl çağırdığını anla. Tüm kütüphane bu mantık üzerine kurulu.
2. Adafruit_ST7735.cpp oku
begin() bul. drawPixel() bul. setAddrWindow() bul. Bu üçü donanım katmanının tamamı.
3. SPI nedir öğren
YouTube: "SPI protocol explained" — 10 dakikalık herhangi bir video.
4. Kendi .h dosyani yaz
Okuduklarından anladığın komutları define olarak yaz. Googlelama, sadece okuduklarından çıkar.
5. Kendi fonksiyonlarını yaz
Önce init, sonra set_window, sonra pixel, sonra fill_rect, sonra line.
6. Arduino IDE'de Verify et
Upload etmeden derleme çalışır.
-------

-------
Hızlı Roadmap
-------
[x] Mevcut ST7735 donanım kutuphanesını temizle ve sadece mega 2560 için çalışacak hale getir
[x] GFX kutuphanesındekı gereksız her şeyi kaldır,cpu döngüsünü azalt,sadece mega 2560 ve tft ekran için gerekenleri minimal şekilde bırak
[ ] 3 Farklı benchmark yaz ve 3 farklı kodu,3 farklı şekilde;Hız,gecikme,cpu döngüsü ve efektifliğini değerlendir
[ ] Gerçek donanımda koştur ve uçtan uca test edip,ona göre aksiyon al
-------
