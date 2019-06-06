//TODO
//Make weapons changable

//PINS:
//2 -- Receiever
//3 -- (transistor) IR LED
//8 -- Reload (button)
//9 -- Trigger (button) 
// 4, 5, 6, 7, 10 -- OLED
//4A, 5A, 6A, 7A -- Life LEDs

#include <IRLibDecodeBase.h>
#include <IRLibSendBase.h> 
#include <IRLib_P01_NEC.h>
#include <IRLib_P02_Sony.h>
#include <IRLib_HashRaw.h>
#include <IRLibCombo.h>


// Display
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

// Gameplay constants
#define HIT_CODE 0x811C9DC5 // 2166136261
#define RESET_CODE 0x10C851AE

#define MAX_LIVES 4

// Weapons
struct Weapon {
  enum FireRate { SEMI = 1, BURST = 3, AUTO = 0 };

  FireRate fireRate;
  int magazineSize;
  int fireDelay; // Delay after each shot (!= pull of trigger)

  int ammoCount;

  Weapon(FireRate fr, int mSize, int fdelay) : fireRate(fr), magazineSize(mSize), fireDelay(fdelay) { ammoCount = magazineSize; }
  Weapon() {}
};
const Weapon Pistol { Weapon::SEMI, 17, 0 };
const Weapon BurstRifle { Weapon::BURST, 31, 50 };
const Weapon AutoRifle { Weapon::AUTO, 25, 50 };

Weapon CurrentWeapon;
// -----

// IR sending and receiving objects
#include <IRLibRecv.h>
IRrecv myReceiver(2);
IRdecode myDecoder;
IRsend mySender;
// -----

// Storage for the recorded code
uint8_t codeProtocol;  // The type of code
uint32_t codeValue;    // The data bits if type is not raw
uint8_t codeBits;      // The length of the code in bits

bool gotOne, gotNew;   // Have we received the first code? Have we have received a new different code from a previous one?
// -----

// Shooting
bool triggerDown;
// -----

// Life counting
int ledsGPIO[] = {A4,A5,A6,A7};
bool ledStatus[] = { 1, 1, 1, 1};
volatile int usedLives = 0;
// -----

void setup() {
  //Receiver
  gotOne=false; gotNew=false;
  codeProtocol=UNKNOWN; 
  codeValue=0;
  // ------

  // Life counting
  for (int i = 0; i < MAX_LIVES; i++ ) {
    pinMode(ledsGPIO[i], OUTPUT); // Define LED pins
  }
  // -----

  //Sender
  pinMode(8, INPUT); // Define the reload button pin
  pinMode(9, INPUT); // Define the trigger (button) pin
  triggerDown = false;
  // -----

  //Interrupts
  attachInterrupt(digitalPinToInterrupt(2), ISR_ReceiveSignal, CHANGE);

  //Gameplay
  CurrentWeapon = Pistol;
  // -----

  // Display
  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.display();
  display.clearDisplay();
  
  Serial.begin(9600);
  myReceiver.enableIRIn(); // Start the receiver
  RefreshLEDS();

}

void loop() {

  if(usedLives >= MAX_LIVES){
    GameOver();
  }
  else if (digitalRead(8) == HIGH){
    CurrentWeapon.ammoCount = CurrentWeapon.magazineSize;
    Serial.println("Reloaded.");
  }
  else{
    // Shooting
    if(CurrentWeapon.fireRate != Weapon::AUTO){
      
      if (digitalRead(9) == HIGH && triggerDown == false) {
        for(int i = 0; i < CurrentWeapon.fireRate; i++){ Shoot(); }
        Serial.println();
        triggerDown = true;
      }
      else if (digitalRead(9) == LOW && triggerDown == true) { triggerDown = false; }
    }
    
    else if (digitalRead(9) == HIGH) { Shoot(); }

    // Display
    display.clearDisplay();
    render_ammo(CurrentWeapon.ammoCount, CurrentWeapon.magazineSize);
    render_health(MAX_LIVES - usedLives);
  }
}


// Stores the received IR code
void storeCode(void) {
  gotNew=true;    gotOne=true;
  codeProtocol = myDecoder.protocolNum;
  
  if (codeProtocol==UNKNOWN) {
    //myDecoder.dumpResults();
    codeValue = myDecoder.value;
  }
  else if (myDecoder.value != REPEAT_CODE) {
    codeValue = myDecoder.value;
    codeBits = myDecoder.bits;
  }
}

void Shoot(){
  if(CurrentWeapon.ammoCount > 0){
    mySender.send(SONY, HIT_CODE, 0);
    Serial.println(F("Sent signal."));
    if(CurrentWeapon.ammoCount == 1) { Serial.println("Out of ammo"); }
    CurrentWeapon.ammoCount--;
    
    delay(CurrentWeapon.fireDelay);
  }
}

void GameOver() {
  display.clearDisplay();

  display.setCursor(0,0);
  display.print("Game Over");
}

void CheckHit(){
   if (codeValue == HIT_CODE && usedLives < MAX_LIVES) {
    ledStatus[usedLives] = 0;
    usedLives++;
    RefreshLEDS();
  }
  
  else if(codeValue == RESET_CODE){
    ResetGame(); 
  }
  Serial.println(codeValue);
}

void RefreshLEDS(){
  for (int i = 0; i < MAX_LIVES; i++ ) {
    analogWrite(ledsGPIO[i], ledStatus[i]);
  }  
}

void ResetGame(){
  usedLives = 0;
  
  CurrentWeapon.ammoCount = CurrentWeapon.magazineSize;
  
  for (int i = 0; i < MAX_LIVES; i++ ) {
    ledStatus[i] = 1;
  }
  
  RefreshLEDS();
}

void ISR_ReceiveSignal()
{
  if (myReceiver.getResults()) {
    myDecoder.decode();
    storeCode();
    CheckHit();
    myReceiver.enableIRIn(); // Re-enable receiver
  }
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
