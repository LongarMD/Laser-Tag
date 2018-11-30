//PINS:
//2 -- Receiever
//3 -- (transistor) IR LED
//9 -- Button

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

//These flags keep track of whether we received the first code 
//and if we have have received a new different code from a previous one.
bool gotOne, gotNew; 

void setup() {
  //Receiver
  gotOne=false; gotNew=false;
  codeProtocol=UNKNOWN; 
  codeValue=0; 

  //Sender
  pinMode(9, INPUT);
  buttonPressed = false;
  
  Serial.begin(9600);
  Serial.println(F("Type any character and press enter. We will send the recorded code."));
  
  myReceiver.enableIRIn(); // Start the receiver
}

// Stores the code for later playback
void storeCode(void) {
  gotNew=true;    gotOne=true;
  codeProtocol = myDecoder.protocolNum;
  Serial.print(F("Received "));
  Serial.print(Pnames(codeProtocol));
  if (codeProtocol==UNKNOWN) {
    Serial.println(F(" saving raw data."));
    myDecoder.dumpResults();
    codeValue = myDecoder.value;
  }
  else {
    if (myDecoder.value == REPEAT_CODE) {
      // Don't record a NEC repeat value as that's useless.
      Serial.println(F("repeat; ignoring."));
    } else {
      codeValue = myDecoder.value;
      codeBits = myDecoder.bits;
    }
    Serial.print(F(" Value:0x"));
    Serial.println(codeValue, HEX);
  }
}

void loop() {
  if (myReceiver.getResults()) {
    myDecoder.decode();
    storeCode();
    myReceiver.enableIRIn(); // Re-enable receiver
  }

  if ((Serial.read() != -1 || digitalRead(9) == HIGH) && buttonPressed == false) {
    mySender.send(SONY,0x10c8e11e, 20);//Sony DVD power A8BCA, 20 bits
    //mySender.send(NEC,0x61a0f00f,0);

    buttonPressed = true;
    Serial.println(F("Sent signal."));
  }
  else if (digitalRead(9) == LOW && buttonPressed == true) { buttonPressed = false; }
}
