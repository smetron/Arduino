#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define DEVICE_NAME         "FCS_Meter_2408"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

unsigned long previousMillis = 0;
int counter = 0;
bool autoRestart = true;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) {
      counter = 0; 
      //pCharacteristic->setValue("data");
    }
    
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++){
          Serial.print(value[i]);

          counter = 0;
          
          if(value[i] == '1'){
            pinMode(2, OUTPUT);
            digitalWrite(2, HIGH);
          } else {
            pinMode(2, OUTPUT);
            digitalWrite(2, LOW);
          }
        }
          
          

        Serial.println();
        Serial.println("*********");
      }
    }
};

  BLECharacteristic *pCharacteristic;
void setup() {
  Serial.begin(115200);

  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("0");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}





void RestartCounter(){
  if (millis() - previousMillis >= 1000) {
    counter++;
    previousMillis = millis();
  }

  if(counter >= 15){
    if(autoRestart){
      digitalWrite(2, LOW);
      
      ESP.restart();  
    }
  }

  Serial.println(counter);
}


String result = "";

void loop() {
  RestartCounter();

  String message = Serial.readStringUntil('\n');
  
  if (message.length() >= 1) {
    result = message;
    result.trim(); 
  }

  Serial.println(result);
  
  char buffer[40];
  sprintf(buffer, "%s", result);
        
  pCharacteristic->setValue(buffer);

  delay(1000);
}
