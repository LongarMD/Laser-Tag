//TODO
//Make weapons changable

//PINS:
//2 -- Receiever
//3 -- (transistor) IR LED
//8 -- Reload (button)
//9 -- Trigger (button)
//10, 11, 12, 13 -- Life LEDs

#include <IRLibDecodeBase.h>
#include <IRLibSendBase.h> 
#include <IRLib_P01_NEC.h>
#include <IRLib_P02_Sony.h>
#include <IRLib_HashRaw.h>
#include <IRLibCombo.h>

// Gameplay
// Constants
#define HIT_CODE 0x10c8e11e // 281600286(dec)
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
int ledsGPIO[] = { 10 , 11 , 12, 13};
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
  CurrentWeapon = AutoRifle;
  // -----
  
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
    mySender.send(NEC, HIT_CODE, 0);
    Serial.println(F("Sent signal."));
    if(CurrentWeapon.ammoCount == 1) { Serial.println("Out of ammo"); }
    CurrentWeapon.ammoCount--;
    
    delay(CurrentWeapon.fireDelay);
  }
}

void GameOver() {
  for (int i = 0; i < MAX_LIVES; i++ ) {
    digitalWrite(ledsGPIO[i], LOW);
  }
  delay(500);
  
  for (int i = 0; i < MAX_LIVES; i++ ) {
    digitalWrite(ledsGPIO[i], HIGH);
  }
  delay(500);
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
}

void RefreshLEDS(){
  for (int i = 0; i < MAX_LIVES; i++ ) {
    digitalWrite(ledsGPIO[i], ledStatus[i]);
  }  
}

void ResetGame(){
  usedLives = 0;
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
