/*
* Arduino Wireless Communication Tutorial
*       Example 1 - Receiver Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN

const byte address[6] = "00001";
//const int rled = 9;

void setup() {
  //pinMode(rled, OUTPUT);
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
  radio.printDetails();
}

void loop() {

  while(radio.available()){
    //char text[32] = "";
    float msg = 0.00;
//    radio.read(&text, sizeof(text));
    radio.read(&msg, sizeof(msg));
    //Serial.println(text);
    //Serial.println(msg);
    Serial.println(msg);
    delay(5);
    
  }
}