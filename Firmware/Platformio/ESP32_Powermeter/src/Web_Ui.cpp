#include <main.h>
#include <Web_Ui.h>

float voltage[3];
float current[3];
double power[3];
float energyWh[3];;
bool  energyRunning[3] = {false,  false, false};
bool reset_status[3] = {false,  false, false};

// Replace with your network credentials
const char* ssid     = "";
const char* password = "";
// Variable to store the HTTP request
String header;
// Set web server port number to 80
WebServer server(80);
void WEB_UI_init(){
  
  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/", HTTP_GET, [](){ server.send(200, "text/html", createHTML()); });
  server.on("/energy", HTTP_POST, handleEnergyCmd);
  server.on("/energy", HTTP_GET,  handleEnergyCmd);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
}
void WEB_UI_Stream(PowerSample &Power_Values){
  for (uint8_t i = 0; i < 3; i++) {
    voltage[i] = Power_Values.Voltage[i];
    current[i] = Power_Values.Current[i];
    power[i]   = Power_Values.Power[i];
    energyWh[i] = Power_Values.energyWh[i];
    Power_Values.counter_state[i] = energyRunning[i];
    if(reset_status[i] == true){
      Power_Values.reset_state[i] = reset_status[i];
      reset_status[i] = false;
    }
  }  
  server.handleClient();
}

void handle_OnConnect() {
  server.send(200, "text/html", createHTML());
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}
void handleEnergyCmd() {
  if (!server.hasArg("ch") || !server.hasArg("cmd")) {
    server.send(400, "text/plain", "Missing ch or cmd");
    return;
  }

  int ch = server.arg("ch").toInt(); 
  String cmd = server.arg("cmd");      
  if (ch < 1 || ch > 3) {
    server.send(400, "text/plain", "ch must be 1..3");
    return;
  }
  int i = ch - 1;

  if (cmd == "start") {
    energyRunning[i] = true;
  } else if (cmd == "stop") {
    energyRunning[i] = false;
  } else if (cmd == "reset") {
    reset_status[i] = true; //Web Ui reset Trigger
  } else {
    server.send(400, "text/plain", "unknown cmd");
    return;
  }
}


String createHTML() {
  String str;
  str.reserve(8000);

  str += "<!DOCTYPE html><html><head>";
  str += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  str += "<meta charset=\"utf-8\">";
  str += "<style>";
  str += "body{font-family:Arial,sans-serif;color:#444;margin:0;padding:16px;background:#f7f7f7;text-align:center;}";
  str += ".container{max-width:1000px;margin:0 auto;}";
  str += ".title{font-size:30px;font-weight:bold;letter-spacing:1px;margin:24px 0 12px;}";
  str += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:16px;margin-top:12px;}";
  str += ".card{background:#fff;border-radius:16px;box-shadow:0 6px 16px rgba(0,0,0,0.06);padding:20px;}";
  str += ".ch-title{font-size:22px;margin:0 0 10px;color:#222;}";
  str += ".metric{display:flex;justify-content:space-between;align-items:baseline;margin:8px 0;}";
  str += ".label{font-size:14px;opacity:.8;}";
  str += ".value{font-size:26px;font-weight:600;}";
  str += ".unit{font-size:14px;margin-left:6px;opacity:.7;}";
  str += ".row{display:flex;gap:8px;justify-content:center;flex-wrap:wrap;margin-top:12px;}";
  str += ".btn{border:none;border-radius:12px;padding:10px 14px;cursor:pointer;box-shadow:0 3px 10px rgba(0,0,0,.08);font-weight:600;}";
  str += ".btn:active{transform:translateY(1px);}";
  str += ".start{background:#e8f5e9;} .stop{background:#ffebee;} .reset{background:#e3f2fd;}";
  str += ".running{color:#2e7d32;font-weight:700;} .stopped{color:#c62828;font-weight:700;}";
  str += ".footer{font-size:12px;opacity:.6;margin-top:20px;}";
  str += "</style>";

  str += "<script>";
  // Auto-refresh the page every 1 second
  str += "setInterval(function(){location.reload();},1000);";
  // Energy command helper
  str += "function sendCmd(ch,cmd){";
  str += "fetch('/energy?ch='+ch+'&cmd='+cmd,{method:'POST'}).then(_=>location.reload()).catch(_=>location.reload());";
  str += "}";
  str += "</script>";

  str += "</head><body><div class=\"container\">";
  str += "<h1 class=\"title\">3-Channel Power Monitor</h1>";
  str += "<div class=\"grid\">";

  for (int i = 0; i < 3; ++i) {
    const int ch = i + 1;
    const bool run = energyRunning[i];

    str += "<div class=\"card\">";
    str += "<h2 class=\"ch-title\">Channel " + String(ch) + "</h2>";

    // Status
    str += "<div class=\"metric\"><span class=\"label\">Energy Integrator</span>";
    str += "<span class=\"" + String(run ? "running" : "stopped") + "\">";
    str += (run ? "RUNNING" : "STOPPED");
    str += "</span></div>";

    // Voltage
    str += "<div class=\"metric\"><span class=\"label\">Voltage</span>";
    str += "<span class=\"value\">" + String(voltage[i], 1) + "<span class=\"unit\"> V</span></span></div>";

    // Current
    str += "<div class=\"metric\"><span class=\"label\">Current</span>";
    str += "<span class=\"value\">" + String(current[i], 3) + "<span class=\"unit\"> A</span></span></div>";

    // Power (W)
    str += "<div class=\"metric\"><span class=\"label\">Power</span>";
    str += "<span class=\"value\">" + String(power[i], 1) + "<span class=\"unit\"> W</span></span></div>";

    // Energy (Wh)
    str += "<div class=\"metric\"><span class=\"label\">Total Energy</span>";
    str += "<span class=\"value\">" + String(energyWh[i], 3) + "<span class=\"unit\"> Wh</span></span></div>";

    // Butonlar
    str += "<div class=\"row\">";
    if (run) {
      str += "<button class=\"btn stop\" onclick=\"sendCmd(" + String(ch) + ",'stop')\">Stop</button>";
    } else {
      str += "<button class=\"btn start\" onclick=\"sendCmd(" + String(ch) + ",'start')\">Start</button>";
    }
    str += "<button class=\"btn reset\" onclick=\"sendCmd(" + String(ch) + ",'reset')\">Reset</button>";
    str += "</div>";

    str += "</div>"; // .card
  }

  str += "</div>"; // .grid
  str += "<div class=\"footer\">Updated at: " + String(millis()/1000) + "s</div>";
  str += "</div></body></html>";

  return str;
}
