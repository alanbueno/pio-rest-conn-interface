#include <WiFi.h>
#include <DNSServer.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include "Utils.h"

const char *const AP_SSID = "ACCESS_POINT_SSID";
const char *const DOMAIN_NAME = "boarddns.com";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

typedef enum
{
  NONE = 0,         /**< initial mode */
  STA_ONLY_ENABLED, /**< after enabling WiFi station mode */
  AP_ONLY_ENABLED,  /**< after enabling WiFi soft-AP mode */
  BOTH_ENABLED,     /**< both modes on */
} enabled_mode_t;

enabled_mode_t enabledConnection = NONE;

WiFiClass *resetConnection()
{
  WiFi.disconnect() && WiFi.mode(WIFI_OFF);

  WiFi.mode(WIFI_AP_STA); //Both sta and ap classes are initiated

  taskWaitInMs(2000);

  return &WiFi;
}

bool spinUpAccessPoint()
{
  customLog("Enabling WiFi AP start");

  bool ok = true;

  WiFi.enableAP(true);

  ok = WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  ok = WiFi.softAP(AP_SSID);

  // if DNSServer's started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  customLog("Starting DNS server");
  ok = dnsServer.start(DNS_PORT, DOMAIN_NAME, apIP);

  if (ok)
    enabledConnection = (enabled_mode_t)(enabledConnection | AP_ONLY_ENABLED);

  customLog("Enabling WiFi AP Result: " + String(ok));

  return ok;
}

WiFiClass *spintUpStationConnection(const char *ssid, const char *passphrase)
{
  customLog("Enabling WiFi.enableSTA");
  if (!WiFi.enableSTA(true))
    return NULL;

  customLog("WiFi.begin");
  if (ssid == nullptr && passphrase == nullptr)
    WiFi.begin();
  else
    WiFi.begin(ssid, passphrase);

  customLog("Wait for connection...");
  if ((WiFi.getMode() & WIFI_MODE_STA) == 0)
  {
    return NULL;
  }
  int i = 0;
  while ((!WiFi.status() || WiFi.status() >= WL_DISCONNECTED) && i++ < 100)
  {
    taskWaitInMs(100);
    esp_task_wdt_reset();
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    customLog("Cound't connect to local Wifi network. Check Wifi credentials and Wifi Network status.");
    WiFi.enableSTA(false);
    enabledConnection = (enabled_mode_t)(enabledConnection & (~STA_ONLY_ENABLED));
    return NULL;
  }

  enabledConnection = (enabled_mode_t)(enabledConnection | STA_ONLY_ENABLED);

  customLog("IP Address: " + WiFi.localIP().toString());

  return &WiFi;
}

bool tearDownStationConnection()
{
  Serial.print("WiFi.enableSTA(false);");
  enabledConnection = (enabled_mode_t)(enabledConnection & (~STA_ONLY_ENABLED)); // seting STA as tasks on loop should stop earlier

  if (!WiFi.enableSTA(false))
  {
    enabledConnection = (enabled_mode_t)(enabledConnection | STA_ONLY_ENABLED); // re-seting as enabled cause it failed
    return false;
  }

  return true;
};

bool tearDownAccessPoint()
{
  Serial.print("WiFi.enableAP(false);");
  enabledConnection = (enabled_mode_t)(enabledConnection & (~AP_ONLY_ENABLED)); // seting AP as tasks on loop should stop earlier

  if (!WiFi.enableAP(false))
  {
    enabledConnection = (enabled_mode_t)(enabledConnection | AP_ONLY_ENABLED); // re-seting as enabled cause it failed
    return false;
  }

  return true;
};

bool hasWifiCredentialsInFlash()
{
  wifi_config_t conf;
  if (esp_wifi_get_config(WIFI_IF_STA, &conf) != ESP_OK)
  {
    const char *ssid = reinterpret_cast<const char *>(conf.sta.ssid);
    customLog("Failure to get the credentials in flash.");
    customLog(ssid);
    return false; // failed to get credentials from flash
  }

  const char *ssid = reinterpret_cast<const char *>(conf.sta.ssid);

  customLog("SSID found in flash: " + String(ssid));

  if (strlen(ssid) == 0)
    return false;
  else
    return true;
}

bool networkTasksOnLoopSta()
{
  return WiFi.status() == WL_CONNECTED;
}

bool networkTasksOnLoopAp()
{
  try
  {
    dnsServer.processNextRequest();
    return true;
  }
  catch (const std::exception &e)
  {
    return false;
  }
}

bool networkTasksOnLoop()
{
  switch (enabledConnection)
  {
  case NONE:
    return true;

  case STA_ONLY_ENABLED:
    return networkTasksOnLoopSta();

  case AP_ONLY_ENABLED:
    return networkTasksOnLoopAp();

  case BOTH_ENABLED:
    return networkTasksOnLoopSta() && networkTasksOnLoopAp();

  default:
    customLog("Couldn't determine which network mode is running, please check!");
    return false;
  }
}