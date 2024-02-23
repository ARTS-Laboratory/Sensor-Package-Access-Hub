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
  byte data[20]; // Adjust the size according to your data size
};

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

  Packet packet;
  int dataIndex = 0;
  String line;

  dataFile.seek(0);

  while (true) {
    char c = dataFile.read();

    // Check for end of file
    if (c == -1) {
      break; // Exit the loop if end of file is reached
    }

    // Check if the character is a newline
    if (c == '\n') {
      // Process the line
      processLine(line, packet, dataIndex);

      // Reset line for the next line of data
      line = "";
    } else {
      // Append character to the current line
      line += c;
    }
  }

  // If there's remaining data in the packet, print it
  if (dataIndex > 0) {
    sendDataPacket(packet);
  }

  dataFile.close();

  Serial.println("Done!");
  int i=1;
  while (i=1) {
    delay(100);
  }
}

void processLine(String line, Packet& packet, int& dataIndex) {
  // Split the line into values
  char* token = strtok(const_cast<char*>(line.c_str()), ",");
  int index = 0;

  // Process each token
  while (token != NULL && index < 20) {
    // Convert token to double and store it in the packet
    packet.data[dataIndex++] = atof(token);

    // Get the next token
    token = strtok(NULL, ",");
    index++;
  }

  // If the packet is full, print its data
  if (dataIndex >= 20) {
    sendDataPacket(packet);
    dataIndex = 0;
  }
}

void sendDataPacket(Packet packet) {
  Serial.println("Packet data:");
  for (int i = 0; i < sizeof(packet.data); i++) {
    Serial.print(packet.data[i]);
    Serial.print(" ");
  }
  Serial.println();
}
