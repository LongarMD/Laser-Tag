#include <Adafruit_SSD1306.h>

#define OLED_CLK 6
#define OLED_MOSI 7
#define OLED_RESET 8
#define OLED_DC 9
#define OLED_CS 10

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC);
  //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(19, 02, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(7, 12, 2017);   // Set the date to June 6th, 2017
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0,0);
  display.display();
}

void loop() {
  display.clearDisplay();

  display.setCursor(0,0);
  display.print("OwO");

  display.display(); 
  delay(1000);

}
