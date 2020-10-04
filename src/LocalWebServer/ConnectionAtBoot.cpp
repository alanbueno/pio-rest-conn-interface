#include "NetworkManager.h"
#include "Routes.h"
#include "Utils.h"

const int MAX_RETRY_ATTEMPTS = 5;
const int RECONNECTION_INTERVAL = 5000;

bool figureConnectionInitialization()
{
  bool ok = true;
  resetConnection();

  if (hasWifiCredentialsInFlash() && spintUpStationConnection(nullptr, nullptr))
  {
    customLog("Credentials found and STA mode connected!");
    ok = registerStationRoutes();
    customLog("Successfully set STA routes");

    return true;
  }

  customLog("No credentials stored or connection failed, starting AP mode!");
  ok = spinUpAccessPoint();
  ok = registerAccessPointRoutes();
  customLog("Successfully set AP mode with routes");

  return ok;
}

void connectionCheckerFunction(void *pvParameters)
{
  bool isConnectionHealthy = figureConnectionInitialization();
  int connAttemptsCount = 0;

  while (connAttemptsCount <= MAX_RETRY_ATTEMPTS)
  {
    // letting the default task for core 0 feed wtd
    // since this task has higher prio we shouldn't block'em
    vTaskDelay(10);

    if (connAttemptsCount == MAX_RETRY_ATTEMPTS)
    {
      isConnectionHealthy = figureConnectionInitialization();
      connAttemptsCount = 0;
      continue;
    }

    if (isConnectionHealthy && networkTasksOnLoop())
      continue;

    if (spintUpStationConnection(nullptr, nullptr) && networkTasksOnLoop())
    {
      customLog("Connection healthy again!");
      isConnectionHealthy = true;
      connAttemptsCount = 0;
      continue;
    }

    isConnectionHealthy = false;
    connAttemptsCount++;

    customLog("Retry attempts: " + String(connAttemptsCount));

    taskWaitInMs(connAttemptsCount * RECONNECTION_INTERVAL);
  }
}

void registerConnectionBackgroundChecker()
{
  TaskHandle_t connectionHealthCheckTask;

  xTaskCreatePinnedToCore(
      connectionCheckerFunction,  /* Task function. */
      "ConnHealthCheck",          /* name of task. */
      10000,                      /* Stack size of task */
      NULL,                       /* parameter of the task */
      2,                          /* priority of the task */
      &connectionHealthCheckTask, /* Task handle to keep track of created task */
      0);                         /* pin task to core 0 */
}