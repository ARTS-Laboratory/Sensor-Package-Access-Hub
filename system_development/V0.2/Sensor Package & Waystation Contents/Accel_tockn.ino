#include <Wire.h>
#include <MPU6050_tockn.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <SPI.h>

MPU6050 mpu(Wire);
File dataFile;
const int chipSelect = 10;
int i = 0;

void setup() {
  
  Serial.begin(9600);

  Serial.println("Start!");
  // Initialize MPU6050
  Wire.begin();
  mpu.begin();
  
  Serial.println("MPU6050 Found!");
  mpu.calcGyroOffsets(true);

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("card initialized.");

  // Create a new file on the SD card
  SD.remove("MPU6050_data.txt");
  File dataFile = SD.open("MPU6050_data.txt", FILE_WRITE);
  //File dataFile = SD.open("MPU6050_data.txt", O_TRUNC);
  if (dataFile) {
    
  } else {
    Serial.println("Error opening file for writing!");
  }
}

void loop() {
  // Read accelerometer and gyroscope data
    i++;
    mpu.update();
    dataFile = SD.open("MPU6050_data.txt", FILE_WRITE);
  //   /* Print out the values */
    //Serial.print("AccelX:");
    Serial.print(mpu.getAccX() - .01);
    Serial.print(", ");
    //Serial.print("AccelY:");
    Serial.print(mpu.getAccY());
    Serial.print(", ");
    //Serial.print("AccelZ:");
    Serial.print(mpu.getAccZ() - .87);
    Serial.print(", ");
    //Serial.println("");
  
    dataFile.print(mpu.getAccX() - .01);
    dataFile.print(" ");
    dataFile.print(mpu.getAccY());
    dataFile.print(" ");
    dataFile.print(mpu.getAccZ() - .87);
    dataFile.print(" ");
    
    dataFile.close();
 

  // Delay for some time (adjust as needed)
  delay(10);
  while (i == 1000) {
    delay(100);
  }
}
