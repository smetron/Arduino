#define DEBUG true

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 18, 17); //18(RX), 17(TX) => AT Module

  Serial.println("Initializing...");
  delay(1000);

  Serial1.println("AT"); //Send AT Command
  UpdateSerial();

  //ModuleStateCheck();

}

void loop()
{
  UpdateSerial();
}

void UpdateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    Serial1.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(Serial1.available()) 
  {
    Serial.write(Serial1.read());//Forward what Software Serial received to Serial Port
  }
}

bool ModuleStateCheck()
{
    int i = 0;
    bool moduleState=false;
    for (i = 0; i < 5; i++)
    {
        String msg = String("");
        msg = SendData("AT", 1000, DEBUG);
        if (msg.indexOf("OK") >= 0)
        {
            Serial.println("AT Module Working");
                moduleState=true;
            return moduleState;
        }

        delay(1000);
    }
    return moduleState;
}

String SendData(String command, const int timeout, boolean debug)
{
    String response = "";
    Serial1.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            response += c;
        }
    }
    if (debug)
    {
        Serial.print(response);
    }
    return response;
}
