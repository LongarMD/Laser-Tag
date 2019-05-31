#include <Adafruit_SSD1306.h>

#define OLED_CLK 4
#define OLED_MOSI 5
#define OLED_RESET 6
#define OLED_DC 7
#define OLED_CS 10

#define SSD1306_128_64

static const int HEART_HEIGHT = 14;
static const int HEART_WIDTH = 16;

static const int HEALTH_X = 24;
static const int HEALTH_Y = 0;

static const unsigned char PROGMEM heart_bmp[] =
{ B00001100, B00110000,
  B00011110, B01111000,
  B00111111, B11111100,
  B01111111, B11111110,
  B11111111, B11111111,
  B11111111, B11111111,
  B01111111, B11111110,
  B00111111, B11111100,
  B00011111, B11111000,
  B00001111, B11110000,
  B00000111, B11100000,
  B00000011, B11000000,
  B00000001, B10000000,
  B00000000, B00000000
};

static const int AMMO_X = 9;
static const int AMMO_Y = 28;

static const unsigned char PROGMEM line_bmp[] =
{ B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11,
  B11, B11
};

static const int LINE_HEIGHT = 12;
static const int LINE_WIDTH = 4;

Adafruit_SSD1306 display(128,64,OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void draw_heart(int width, int height, int flag=1);

void render_ammo(int ammo, int max_ammo, int bars = 20);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC);
  Serial.begin(9600);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.display();
  display.clearDisplay();
}

void loop() {
  
  render_ammo(4, 32);
  render_health(4);

}


void draw_heart(int x, int y, int flag=1) {
  display.drawBitmap(
    x,
    y,
    heart_bmp, HEART_WIDTH, HEART_HEIGHT, flag);
  display.display();
}

void draw_line(int x, int y, int flag=1) {
  display.drawBitmap(
    x,
    y,
    line_bmp, LINE_WIDTH * 2, LINE_HEIGHT, flag);
  display.display();
}

void render_health(int health){
    for(int i=0; i < health; i++){
        int spacing = 0;
        if(i > 0){spacing=4;}
        draw_heart(HEALTH_X + (HEART_WIDTH + spacing) * i, HEALTH_Y);
       }
}

void render_ammo(int ammo, int max_ammo, int bars = 20){
    int ammo_bars = round((float(ammo) / float(max_ammo)) * float(bars));

    for(int i=0; i < ammo_bars; i++){
        int spacing = 0;
        if(i > 0){spacing=1;}
        draw_line(AMMO_X + (LINE_WIDTH + spacing) * i, AMMO_Y);
    }

    display.setCursor(50, 32 + LINE_HEIGHT + 2);
    display.print(ammo);
}
