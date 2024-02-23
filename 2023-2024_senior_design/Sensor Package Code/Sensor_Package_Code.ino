
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <SPI.h>
#include "printf.h"
#include <RF24.h>

#define CE_PIN 7
#define CSN_PIN 8

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
const byte address[6] = "00001";
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
//bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit

// Used to control whether this node is sending or receiving
//bool role = true;  // true = TX role, false = RX role

MPU6050 mpu(Wire);
File dataFile;
const int chipSelect = 1;
int i = 0;

const int dataPoints = 3000;

float buffer;

char dataBuffer[5];
char data[dataPoints][5];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  mpu.begin();

  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
    }
  }
  Serial.println("card initialized.");

  // char input = 0;
  // radioNumber = input == 1;
  // Serial.print(F("radioNumber = "));
  // Serial.println((int)radioNumber);

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_MAX);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(1);  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address);  // always uses pipe 0
  radio.stopListening();
  // set the RX address of the TX node into a RX pipe
  //radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1

  // if (role) {
  //   radio.stopListening();  // put radio in TX mode
  // } else {
  //   Serial.println("listening...");
  //   radio.startListening();  // put radio in RX mode
  // }

  //populate data array with zeros
  for (int i = 0; i < dataPoints; i++) {
    for(int j = 0; j < 5; j++){
      data[i][j] = '0';
    }
  }

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("card initialized.");

  // Create a new file on the SD card
  SD.remove("MPU6050_data.csv");
  File dataFile = SD.open("MPU6050_data.csv", FILE_WRITE);
  //File dataFile = SD.open("MPU6050_data.txt", O_TRUNC);
  if (dataFile) {
    
  } else {
    Serial.println("Error opening file for writing!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long start_timer = micros();
  dataFile = SD.open("MPU6050_data.csv", FILE_WRITE);

  //Start Data Collection Loop
  for (int i = 1; i <= dataPoints; i++) {
      mpu.update();
      if (i % 3 == 1) {
        buffer = mpu.getAccX() - .06;
      }
      else if (i % 3 == 2) {
        buffer = mpu.getAccY() - .03;
      }
      else if (i % 3 == 0) {
        buffer = mpu.getAccZ() - .88;
        delay(2);
      }
      Serial.print(buffer);
      Serial.print(", ");
      dtostrf(buffer, 5, 2, dataBuffer);
        for(int j = 0; j < 5; j++){
          if (j == 0 && dataBuffer[j] == ' ') {
            //iterate with no changes
          }
          else {
            data[i-1][j] = dataBuffer[j];
          }
        }
        dataFile.print(buffer);
    }
    //End Data Collection Loop

    unsigned long end_timer = micros();

    Serial.println();
    Serial.println();
    Serial.println("--------------------------------------------------");
    Serial.println();
    
    Serial.print(F("Time to record = "));
    Serial.println(end_timer - start_timer);

    // start write data to console
    for (int i = 0; i < dataPoints; i++) {
      for(int j = 0; j < 5; j++){
        Serial.print(data[i][j]);
        
      }
      Serial.print(", ");
    }
    //end write data to console
    Serial.println();

    dataFile.close();

    radio.flush_tx();

    int n = 0;
    int failures = 0;
    unsigned long start_timer1 = micros();

    Serial.println("Attempting to transmit");
    
    while  (n < dataPoints){
      //for (int i = 0; i<dataPoints; i++){
        for (int j = 0; j < 5; j++) {
          dataBuffer[j] = data[n][j];
          Serial.println(dataBuffer);
        }
        radio.write(&dataBuffer, sizeof(dataBuffer));
        n++;
      //   if (!radio.write(&dataBuffer, sizeof(dataBuffer))) {
      //   failures++;
      //   radio.reUseTX();
      //   Serial.println(dataBuffer);
      // } else {
      //   n++;
      //   }
      //}
    }
    unsigned long end_timer1 = micros();

    Serial.print(F("Time to transmit = "));
    Serial.print(end_timer1 - start_timer1);  // print the timer result
    Serial.print(F(" us with "));
    Serial.print(failures);  // print failures detected
    Serial.println(F(" failures detected"));
    while (1) {
      //END
    }

}
