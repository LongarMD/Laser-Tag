#include <Adafruit_SSD1306.h>

#define OLED_CLK 9
#define OLED_MOSI 10
#define OLED_RESET 11
#define OLED_DC 12
#define OLED_CS 13

#define SSD1306_128_64
// #define SSD1306_128_32
// #define SSD1306_96_16
static const unsigned char PROGMEM logo_bmp[] =
{ B00001100, B00110000,
  B00011110, B01111000,
  B00111111, B11111100,
  B00111111, B11111100,
  B01111111, B11111110,
  B01111111, B11111110,
  B01111111, B11111110,
  B00111111, B11111100,
  B00111111, B11111100,
  B00011111, B11111000,
  B00001111, B11110000,
  B00000111, B11100000,
  B00000011, B11000000,
  B00000001, B10000000
};

Adafruit_SSD1306 display(128,64,OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextColor(BLACK);
  display.setTextSize(4);
  display.setCursor(10,0);
  display.display();
}

void loop() {
  display.clearDisplay();
  testdrawbitmap(20,20,14,16,1);
  delay(1000);
}


void testdrawbitmap(int WIDTH, int HEIGHT, int bmpHeight, int bmpWidth, int flag) {
  display.drawBitmap(
    WIDTH,
    HEIGHT,
    logo_bmp, bmpWidth, bmpHeight, flag);
  display.display();
  delay(1000);
}
