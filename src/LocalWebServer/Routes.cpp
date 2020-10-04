#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "NetworkManager.h"
#include "Utils.h"
#include "Routes.h" // due to the route registers redundance

AsyncWebServer server(80);

std::string staRoutesPrefix = "/sta";
std::string apRoutesPrefix = "/ap";
std::string switchConnectionPath = "/switch-connection-to";

std::string inApEnableStaPath = apRoutesPrefix + switchConnectionPath + staRoutesPrefix;
std::string inStaEnableAP = staRoutesPrefix + switchConnectionPath + apRoutesPrefix;

const char *contentType = "application/json";

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, contentType, "{\"message\":\"Not found\"}");
}

void registerCommonRouteHandlers(std::string &prefix, std::string &inversePath, AsyncWebServer &server)
{
  server.on((prefix + "/ping").c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, contentType, "{\"message\":\"pong\"}");
  });

  server.on(inversePath.c_str(), HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(409, contentType, "{\"message\":\"you're trying to enable the current active mode, please check\"}");
  });
}

bool registerStationRoutes()
{
  server.reset();
  registerCommonRouteHandlers(staRoutesPrefix, inApEnableStaPath, server);

  server.on(inStaEnableAP.c_str(), HTTP_POST, [&](AsyncWebServerRequest *request) {
    bool ok = true;

    ok = spinUpAccessPoint();

    ok = registerAccessPointRoutes();

    if (!ok)
    {
      request->send(500, contentType, "{\"message\":\"Something went wrong\"}");
      return;
    }

    request->send(200, contentType, "{\"message\":\"Ap mode set successfully!\"}");

    taskWaitInMs(2000);
    tearDownStationConnection();
  });

  server.onNotFound(notFound);

  server.begin();

  return true;
}

bool registerAccessPointRoutes()
{
  server.reset();
  registerCommonRouteHandlers(apRoutesPrefix, inStaEnableAP, server);

  AsyncCallbackJsonWebHandler *enableStaHandler = new AsyncCallbackJsonWebHandler(inApEnableStaPath.c_str(), [&](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<100> payload;
    payload = json.as<JsonObject>();

    const char *ssid = payload["ssid"];
    const char *password = payload["password"];

    customLog(String("SSID as a req param: ") + ssid);

    customLog("Startint STA");

    auto wiFi = spintUpStationConnection(ssid, password);

    if (!wiFi || !registerStationRoutes())
    {
      request->send(500, contentType, "{\"message\":\"Something went wrong\"}");
      return;
    }

    payload["ip"] = wiFi->localIP().toString();
    payload["message"] = "Started Sta mode";
    payload.remove("password");

    String response;
    serializeJson(payload, response);
    request->send(200, contentType, response);

    taskWaitInMs(2000);
    tearDownAccessPoint();
  });

  server.addHandler(enableStaHandler);

  server.onNotFound(notFound);

  server.begin();

  return true;
}