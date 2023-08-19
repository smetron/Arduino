#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define DEVICE_NAME         "FCS_Meter_2408"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b" //LED Control Service
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++){
          Serial.print(value[i]);

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

  pCharacteristic->setValue("Sample Data");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {

  int cnt = 123456789;

  char buffer[40];
  sprintf(buffer, "%d", cnt);
        
  pCharacteristic->setValue(buffer);

  delay(2000);
}