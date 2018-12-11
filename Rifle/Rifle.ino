//PINS:
//2 -- Receiever
//3 -- (transistor) IR LED
//9 -- Button
//10, 11, 12, 13 -- Life LEDs

#include <IRLibDecodeBase.h>
#include <IRLibSendBase.h>  
#include <IRLib_P01_NEC.h>
#include <IRLib_P02_Sony.h>
#include <IRLib_HashRaw.h>    //We need this for IRsendRaw
#include <IRLibCombo.h>

IRdecode myDecoder;
IRsend mySender;

bool buttonPressed;

// Include a receiver either this or IRLibRecvPCI or IRLibRecvLoop
#include <IRLibRecv.h>
IRrecv myReceiver(2);

// Storage for the recorded code
uint8_t codeProtocol;  // The type of code
uint32_t codeValue;    // The data bits if type is not raw
uint8_t codeBits;      // The length of the code in bits

//DEFINICIJA IR KOD
#define HIT_CODE 281600286
#define MAX_LIVES 4

int ledsGPIO[] = { 10 , 11 , 12, 13};
bool ledsStatus[] = { 1, 1, 1, 1};
volatile int usedLives = 0;

//These flags keep track of whether we received the first code 
//and if we have have received a new different code from a previous one.
bool gotOne, gotNew; 

void setup() {
  //Receiver
  gotOne=false; gotNew=false;
  codeProtocol=UNKNOWN; 
  codeValue=0;

  for (int i = 0; i < MAX_LIVES; i++ ) {
    pinMode(ledsGPIO[i], OUTPUT);
  }

  //Sender
  pinMode(9, INPUT);
  buttonPressed = false;

  //Interrupt
  attachInterrupt(digitalPinToInterrupt(2), ISR_ReceiveSignal, CHANGE);
  
  Serial.begin(9600);
  Serial.println(F("Type any character and press enter. We will send the recorded code."));
  
  myReceiver.enableIRIn(); // Start the receiver
  RefreshLEDS();
}

void loop() {

  if(usedLives >= MAX_LIVES){
    GameOver();
  }
  
  // Shooting
  if (digitalRead(9) == HIGH && buttonPressed == false) {
    mySender.send(NEC,0x10c8e11e, 0);//Sony DVD power A8BCA, 20 bits
    //mySender.send(NEC,0x61a0f00f);
    
    buttonPressed = true;
    Serial.println(F("Sent signal."));
  }
  else if (digitalRead(9) == LOW && buttonPressed == true) { buttonPressed = false; }
}


// Stores the code for later playback
void storeCode(void) {
  gotNew=true;    gotOne=true;
  codeProtocol = myDecoder.protocolNum;

  // Ker uporabljamo interrupte, serial printov ni pametno uporabljati
  //Serial.print(F("Received "));
  //Serial.print(Pnames(codeProtocol));
  
  if (codeProtocol==UNKNOWN) {
    //Serial.println(F(" saving raw data."));
    myDecoder.dumpResults();
    codeValue = myDecoder.value;
  }
  else {
    if (myDecoder.value == REPEAT_CODE) {
      // Don't record a NEC repeat value as that's useless.
      //Serial.println(F("repeat; ignoring."));
    } else {
      codeValue = myDecoder.value;
      codeBits = myDecoder.bits;
    }
    Serial.print(F(" Value:0x"));
    Serial.println(codeValue, HEX);
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
  //TODO: Koda za "respawn"
  
   if (codeValue == HIT_CODE && usedLives < MAX_LIVES) {
    ledsStatus[usedLives] = 0;
    usedLives++;
  }
  RefreshLEDS();
}

void RefreshLEDS(){
  //Refresh LEDs
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
