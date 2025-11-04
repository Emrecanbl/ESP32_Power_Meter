// Load required libraries
#include <WiFi.h>
#include <WebServer.h>

void WEB_UI_init();
void WEB_UI_Stream(PowerSample &Power_Values);
void handle_OnConnect();
void handle_NotFound();
void handleEnergyCmd();
String createHTML();
