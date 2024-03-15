#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <SPI.h>
#include <stdlib.h>

MPU6050 mpu(Wire);
File dataFile;
const int chipSelect = 1;
int i = 0;
const int dataPoints = 1000;
float buffer;

struct Packet {
  String accel; // Adjust the size according to your data size
};

Packet packet;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

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

}

void loop() {
  // put your main code here, to run repeatedly:

  unsigned long start_timer = micros();
  dataFile = SD.open("MPU6050_data.csv", FILE_WRITE);

  for (int i = 0; i < dataPoints; i++) {
      mpu.update();
      Serial.print(mpu.getAccX() - .01);
      Serial.print(", ");
      Serial.print(mpu.getAccY());
      Serial.print(", ");
      Serial.print(mpu.getAccZ() - .87);
      Serial.println(", ");
      dataFile.print(mpu.getAccX() - .01);
      dataFile.print(", ");
      dataFile.print(mpu.getAccY());
      dataFile.print(", ");
      dataFile.print(mpu.getAccZ() - .87);
      dataFile.println(", ");
      delay(8);
  }
    //End Data Collection Loop
  unsigned long end_timer = micros();
  Serial.println(end_timer - start_timer);
  i=0;
    
  // while (dataFile.available()) {
  //   packet.data[i++] = dataFile.read();
  //   if (i >= sizeof(packet.data)) {
  //     // Send the packet when it's full
  //     sendDataPacket(packet);
  //     i = 0;
  //   }
  // }

  // if (i > 0) {
  //   sendDataPacket(packet);
  // }

  dataFile.seek(0);
  for (int i = 0; i < dataPoints; i++) {
    packet.accel = readAccelSensorVoltage();
    printSensorData(packet);
  }

  // If there's remaining data in the packet, print it

  dataFile.close();

  Serial.println("Done!");
  int i=1;
  while (i = 1) { 
    delay(100);
  }
}

String readAccelSensorVoltage() {
  String buffer = dataFile.read();
  return buffer;
}

void printSensorData(const Packet& packet) {
  
  Serial.println(packet.accel); // Display float with 2 decimal places
  
}
