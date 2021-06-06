
#include <stdlib.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);

}

void togglePin(int pin){
      int i;
      i= digitalRead(pin);
      Serial.print(pin);
      
      if (i==HIGH){
         digitalWrite(pin,LOW);
         Serial.write(" low\n");
      } else {
         digitalWrite(pin,HIGH);
         Serial.write(" high\n");
      }
}

void loop() {
  // put your main code here, to run repeatedly:
  int s[] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
  char inByte;
   if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    if (inByte=='4') {
      togglePin(4);
      
    }
        if (inByte=='5') {
      togglePin(5);
      
    }
        if (inByte=='6') {
      togglePin(6);
      
    }
        if (inByte=='7') {
      togglePin(7);
      
    }
        if (inByte=='8') {
      togglePin(8);
      
    }
        if (inByte=='9') {
      togglePin(9);
      
    }
        if (inByte=='a') {
      togglePin(10);
      
    }
    if (inByte=='b') {
      togglePin(11);
    }
    if (inByte=='p') {
      for (int x=4;x<12;x++){
        Serial.print(x);
        if (digitalRead(x)==HIGH){
          Serial.println(" high");
        } else {
          Serial.println(" low");
        }
        
      }
    }
   }
}
