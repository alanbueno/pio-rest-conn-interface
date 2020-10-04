#include <Arduino.h>
#include "ConnectionAtBoot.h"
#include "Utils.h"

void setup()
{
  Serial.begin(115200);
  customLog("\nSetup!");

  registerConnectionBackgroundChecker();
}

void loop()
{
}