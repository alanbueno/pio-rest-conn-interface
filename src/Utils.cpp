#include <Arduino.h>

void customLog(const String &message)
{
  // if (REMOTE_LOG_ENABLED) // key in build_flags
  // {
  //   // will call remote logger api
  //   return;
  // }

  Serial.println(message);
}

void taskWaitInMs(const int &millis)
{
  vTaskDelay(pdMS_TO_TICKS(millis));
}