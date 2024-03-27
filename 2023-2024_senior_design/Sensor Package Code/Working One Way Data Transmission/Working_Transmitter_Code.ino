#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SD.h>
#include <SPI.h>
#include <stdlib.h>
#include <RF24.h>

MPU6050 mpu(Wire);
File dataFile;
const int chipSelect = 1;
int i = 0;
const int dataPoints = 1000;

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);

const byte address[6] = "00001";

float myVar1;
float myVar2;
float myVar3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }
  
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX); //PICK UP WORK HERE
  radio.stopListening();
  radio.printDetails();




  Serial.println("Start!");
  // Initialize MPU6050
  Wire.begin();
  mpu.begin();

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("card initialized.");

  // Create a new file on the SD card
  SD.remove("MPU6050_data.csv");
  File dataFile = SD.open("MPU6050_data.csv", FILE_WRITE);

  if (dataFile) {
    
  } else {
    Serial.println("Error opening file for writing!");
  }

  unsigned long start_timer = micros();

  for (int i = 0; i < dataPoints; i++) {
      mpu.update();
      Serial.print(mpu.getAccX() - .01);
      Serial.print(", ");
      Serial.print(mpu.getAccY());
      Serial.print(", ");
      Serial.print(mpu.getAccZ() - .87);
      Serial.println(", ");
      dataFile.print(mpu.getAccX() - .01);
      dataFile.print(",");
      dataFile.print(mpu.getAccY());
      dataFile.print(",");
      dataFile.print(mpu.getAccZ() - .87);
      dataFile.print(",");
      delay(8);
  }
    //End Data Collection Loop

  unsigned long end_timer = micros();
  Serial.println(end_timer - start_timer);

  dataFile.close();

}

void loop() {
  // put your main code here, to run repeatedly:
  dataFile = SD.open("MPU6050_data.csv");
     int failures = 0;

     while (dataFile.available()) 
       {
        myVar1 = dataFile.parseFloat();
        myVar2 = dataFile.parseFloat();
        myVar3 = dataFile.parseFloat();

        radio.flush_tx();
        while(!radio.write(&myVar1, sizeof(myVar1))) {
          failures++;
          radio.reUseTX();
          delay(500);
        }
        while(!radio.write(&myVar2, sizeof(myVar2))) {
          failures++;
          radio.reUseTX();
          delay(500);
        }
        while(!radio.write(&myVar3, sizeof(myVar3))) {
          failures++;
          radio.reUseTX();
          delay(500);
        }

        Serial.println(myVar1);
        Serial.println(myVar2);
        Serial.println(myVar3);
      }
     
     Serial.print(failures);  // print failures detected


  
  dataFile.close();

  Serial.println("Done!");
  int k=1;
  while (k = 1) { 
    delay(100);
  }
}

