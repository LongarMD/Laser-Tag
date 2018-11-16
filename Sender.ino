//PINS
//3 -- (transistor) IR LED
//9 -- Button

#include <IRLibSendBase.h>
#include <IRLib_P01_NEC.h>
#include <IRLib_P02_Sony.h>
#include <IRLibCombo.h>

IRsend mySender;

bool buttonPressed;

void setup() {
  pinMode(9, INPUT);
  buttonPressed = false;
  
  Serial.begin(9600);
  Serial.println(F("Every time you press a key is a serial monitor we will send."));
}

void loop() {
  if ((Serial.read() != -1 || digitalRead(9) == HIGH) && buttonPressed == false) {
    mySender.send(SONY,0xa8bca, 20);//Sony DVD power A8BCA, 20 bits
    //mySender.send(NEC,0x61a0f00f,0);//NEC TV power button=0x61a0f00f

    buttonPressed = true;
    Serial.println(F("Sent signal."));
  }
  else if (digitalRead(9) == LOW && buttonPressed == true) { buttonPressed = false; }
}
