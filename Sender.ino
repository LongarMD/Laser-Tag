//PINS
//11 -- IR LED

#include <IRLibSendBase.h>
#include <IRLib_P01_NEC.h>
#include <IRLib_P02_Sony.h>
#include <IRLibCombo.h>

IRsend mySender;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Every time you press a key is a serial monitor we will send."));
}

void loop() {
  if (Serial.read() != -1) {
    //send a code every time a character is received from the 
    // serial port. You could modify this sketch to send when you
    // push a button connected to an digital input pin.
    //Substitute values and protocols in the following statement
    // for device you have available.
    mySender.send(SONY,0xa8bca, 20);//Sony DVD power A8BCA, 20 bits
    //mySender.send(NEC,0x61a0f00f,0);//NEC TV power button=0x61a0f00f
    Serial.println(F("Sent signal."));
  }
}
