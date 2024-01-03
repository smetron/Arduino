/*
  ESP32-based BLE Receiver for Cap
  Send Cap Mac Address over serial terminal with newline like d7:07:f9:5f:60:e3\n
  Pin 1 => HIGH and LOW for open and close
  Pin 2 => Tempring while the Cap is closed
  Pin 3 => For Detecting Ibeacons nearby starting with device name FCS
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "SPIFFS.h"

int scanTime = 3; //In seconds
BLEScan* pBLEScan;

String CapAddress = "d8:07:f9:5f:60:e3";

void SetPin(int pin, byte output){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, output);
}

bool tampering = false;
int counter = 0;


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String deviceName = advertisedDevice.getName().c_str(); 
      String deviceAddress = advertisedDevice.getAddress().toString().c_str();
      
      if(deviceAddress.startsWith(CapAddress)) {
        std::string strManufacturerData = advertisedDevice.getManufacturerData();
    
        uint8_t cManufacturerData[25];
        strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);

        Serial.print(cManufacturerData[0], HEX);
        Serial.print(cManufacturerData[1], HEX);
        Serial.print(cManufacturerData[2], HEX);
        Serial.print(cManufacturerData[3], HEX);
        Serial.print(cManufacturerData[4], HEX);
        Serial.print(cManufacturerData[5], HEX);
        Serial.print(cManufacturerData[6], HEX);
        Serial.print(cManufacturerData[7], HEX);
        Serial.print("_Length:");
        Serial.print(strManufacturerData.length());

        Serial.print(' ');

        //Cap Closed Normal Position (Cap is closed, and there is no tampering)
        if (strManufacturerData.length() == 8 && cManufacturerData[7] == 0x73 && cManufacturerData[6] == 0x6C)
        {
            Serial.print("Closed, ");
            SetPin(2, LOW);
        }

        
        //Tamper Detection (Cap is closed, and someone is tampering the cap)
        if (strManufacturerData.length() == 8 && cManufacturerData[7] == 0x72 && cManufacturerData[6] == 0x6C)
        {
            Serial.print("Closed Tempering, ");
            SetPin(2, LOW);
            tampering = true;
        }

        if(tampering == true) {
          SetPin(1, HIGH);
          tampering = false;
        }
        else {
          SetPin(1, LOW);
        }

        //Cap Opened (Cap is opened, and there is no tempering)
        if (strManufacturerData.length() == 8 && cManufacturerData[7] == 0x73 && cManufacturerData[6] == 0x75)
        {
            Serial.print("Opened, ");
            SetPin(2, HIGH);
        }
        
        //Cap Open (Cap is opened, and someone is tampering the cap)
        if (strManufacturerData.length() == 8 && cManufacturerData[7] == 0x72 && cManufacturerData[6] == 0x75)
        {
            Serial.print("Opened Tempering, ");
            SetPin(2, HIGH);
        }

        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      }

      
      if(deviceName.startsWith("FCS"))
      {
        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
        SetPin(3, HIGH);
        counter = 0;
      } else {
        counter++;
      }

     // Serial.println(counter);

      if(counter >= 30){
        Serial.println("No FCS Detected");
        SetPin(3, LOW);

        counter = 0;
      }
    }
};


void setup()
{
  Serial.begin(115200);
  Serial.println("FCS Cap Receiver V1.0");


 if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }

  ReadFiles();


  if(ReadFile("/CAP") != ""){
    CapAddress = ReadFile("/CAP");  
  }
  

  Serial.println("CAP ID: " + CapAddress);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}


void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory


  String message = Serial.readStringUntil('\n');
  message.trim();

  if (message.length() == 17 && message.charAt(2) == ':') {
    CreateFile("/CAP", message);

    delay(2000);
    ESP.restart();
  }

  delay(1000);
}



String ReadFile(String path) {
    String result = "";

    File file = SPIFFS.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
    }
    while(file.available()){
        result = file.readString();
    }
    file.close();

    result.trim();

    return result;
}

void CreateFile(String filename, String message) {
  File file = SPIFFS.open(filename, FILE_WRITE);

  if(!file){
      Serial.println("There was an error opening the file for writing");
      return;
  }

  if(file.print(message)) {
      Serial.println("File " + filename + " set to " +  message);
  }else {
      Serial.println("File write failed");
  }

  file.close();
}

void ReadFiles() {
  Serial.println("Reading Files");

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  while(file) {

      // Serial.print("FILE: ");
      // Serial.println(file.name());

      while(file.available()){
          Serial.print(file.name());
          Serial.println(',' + file.readString());
      }
      file = root.openNextFile();
  }

  file.close();

  Serial.println("Reading Finished");
}
