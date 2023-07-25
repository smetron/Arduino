#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;

#include <bluefruit.h>

#define MANUFACTURER_ID   0x0059

uint8_t beaconUuid[16] =
{
  0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
  0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
};

// A valid Beacon packet consists of the following information:
// UUID, Major, Minor, RSSI @ 1M
BLEBeacon beacon(beaconUuid, 0x0102, 0x0304, -54);

void setup()
{
    Serial.begin(115200);

    //Connect to VL53L0X Sensor
    Wire.begin();

    sensor.init();
    sensor.setTimeout(500);
    sensor.setMeasurementTimingBudget(200000);
    //END

    Bluefruit.begin();

    Bluefruit.setName("CapSense");



    // off Blue LED for lowest power consumption
    Bluefruit.autoConnLed(false);
    Bluefruit.setTxPower(0);    // Check bluefruit.h for supported values

    // Manufacturer ID is required for Manufacturer Specific Data
    beacon.setManufacturer(MANUFACTURER_ID);

    // Setup the advertising packet
    startAdv();

    // Suspend Loop() to save power, since we didn't have any code there
    //suspendLoop();
}

void startAdv(void)
{
    Bluefruit.Advertising.setBeacon(beacon);

    Bluefruit.ScanResponse.addName();

    /* Start Advertising
     * - Enable auto advertising if disconnected
     * - Timeout for fast mode is 30 seconds
     * - Start(timeout) with timeout = 0 will advertise forever (until connected)
     *
     * Apple Beacon specs
     * - Type: Non connectable, undirected
     * - Fixed interval: 100 ms -> fast = slow = 100 ms
     */
     //Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_ADV_NONCONN_IND);
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(160, 160);    // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
    Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop()
{
    //Read the distance measurement  
    uint16_t reading = sensor.readRangeSingleMillimeters();
  
    Serial.println(reading);

    if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

    BLEBeacon bb(beaconUuid, 1234, reading, -60);

    Bluefruit.Advertising.setBeacon(bb);

    delay(1000);
}
