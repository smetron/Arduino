#include <esp_task_wdt.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Configuring WDT...");
  
  esp_task_wdt_init(10, true); //Set watchdog timer to 10 sec, enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

void loop() {
  if(millis() >= 20000){
    delay(15000);//delay for 15 seconds so the watchdog can be triggered to restart the system
  }
  
  esp_task_wdt_reset(); //reset watchdog timer
  
  Serial.println(millis());
  delay(1000);
}
