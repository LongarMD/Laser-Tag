//PINS:
//2 -- Receiever
//3 -- (transistor) IR LED
//9 -- Button
//10, 11, 12, 13 -- Life LEDs

#include <IRLibDecodeBase.h>
#include <IRLibSendBase.h> 
#include <IRLib_P01_NEC.h>
#include <IRLib_P02_Sony.h>
#include <IRLib_HashRaw.h>
#include <IRLibCombo.h>

// Gameplay constants
#define HIT_CODE 281600286
#define HIT_CODE_HEX 0x10c8e11e
#define MAX_LIVES 4
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

//These flags keep track of whether we received the first code 
//and if we have have received a new different code from a previous one.
bool gotOne, gotNew;
// -----

// Shooting
bool triggerDown;
// -----

// Life counting
int ledsGPIO[] = { 10 , 11 , 12, 13};
bool ledsStatus[] = { 1, 1, 1, 1};
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
  pinMode(9, INPUT); // Define the trigger (button) pin
  triggerDown = false;
  // -----

  //Interrupts
  attachInterrupt(digitalPinToInterrupt(2), ISR_ReceiveSignal, CHANGE);
  
  Serial.begin(9600);
  myReceiver.enableIRIn(); // Start the receiver
  RefreshLEDS();
}

void loop() {

  if(usedLives >= MAX_LIVES){
    GameOver();
  }
  
  // Shooting
  if (digitalRead(9) == HIGH && triggerDown == false) {
    mySender.send(NEC, HIT_CODE_HEX, 0);
    triggerDown = true;
    
    Serial.println(F("Sent signal."));
  }
  else if (digitalRead(9) == LOW && triggerDown == true) { triggerDown = false; }
}


// Stores the received IR code
void storeCode(void) {
  gotNew=true;    gotOne=true;
  codeProtocol = myDecoder.protocolNum;
  
  if (codeProtocol==UNKNOWN) {
    myDecoder.dumpResults();
    codeValue = myDecoder.value;
  }
  else if (myDecoder.value != REPEAT_CODE) {
    codeValue = myDecoder.value;
    codeBits = myDecoder.bits;
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
    ledsStatus[usedLives] = 0;
    usedLives++;
  }
  RefreshLEDS();
}

void RefreshLEDS(){
  for (int i = 0; i < MAX_LIVES; i++ ) {
    digitalWrite(ledsGPIO[i], ledsStatus[i]);
  }  
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
