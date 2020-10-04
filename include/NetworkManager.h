#pragma once

#include <WiFi.h>

WiFiClass *resetConnection();
bool spinUpAccessPoint();
WiFiClass *spintUpStationConnection(const char *ssid, const char *passphrase);
bool networkTasksOnLoop();
bool hasWifiCredentialsInFlash();
bool tearDownStationConnection();
bool tearDownAccessPoint();