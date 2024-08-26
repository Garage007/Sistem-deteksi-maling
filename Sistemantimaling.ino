#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>

const char* ssid = "Pixel_5272";            // SSID WiFi
const char* password = "hhhhhhhh";          // Password WiFi
const char* mqtt_server = "broker.emqx.io"; // Alamat server MQTT

const int lampPin = D4;     // Pin lampu
const int buzzerPin = D7;   // Pin buzzer
const int switchPin = D1;   // Pin magnetic switch

WiFiClient espClient;
PubSubClient mqttClient(espClient);
ESP8266WebServer server(80);

bool systemArmed = false; // Status sistem antimaling
bool buzzerOn = false;    // Status buzzer aktif atau tidak

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // Debounce delay 50ms

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  if (message == "ON") {
    systemArmed = true;
    digitalWrite(lampPin, HIGH); // Nyalakan lampu saat sistem diaktifkan
  } else if (message == "OFF") {
    systemArmed = false;
    digitalWrite(lampPin, LOW);  // Matikan lampu saat sistem dimatikan
    digitalWrite(buzzerPin, LOW); // Matikan buzzer
    buzzerOn = false;
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("mqttx_7ygjfhtfubjyfytth")) {
      Serial.println("connected");
      mqttClient.subscribe("pintu/deteksimaling"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleRoot() {
  String html = "<!doctype html>";
  html += "<html lang=\"en\">";
  html += "<head>";
  html += "<meta charset=\"utf-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>Antimaling Control</title>";
  html += "<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\">";
  html += "<style>";
  html += "*, *:before, *:after { box-sizing: border-box; }";
  html += "html { font-size: 18px; }";
  html += "body { font-family: \"Open Sans\", sans-serif; font-size: 1em; line-height: 1.6; background: #dbdbdb; }";
  html += "body > div { position: absolute; top: 0; bottom: 0; right: 0; left: 0; display: flex; align-items: center; justify-content: center; }";
  html += "label { transform: scale(1.5); display: block; width: 160px; height: 80px; border-radius: 40px; background: linear-gradient(to bottom, #9e9e9e 30%, #f4f4f4); box-shadow: 0 2px 0 0 #fff, 0 -2px 0 0 #969494; position: relative; -webkit-tap-highlight-color: rgba(255, 255, 255, 0); }";
  html += "label input { display: none; }";
  html += "label div { display: block; width: 120px; height: 50px; position: absolute; left: 50%; top: 50%; transform: translate(-50%, -50%); background: linear-gradient(to bottom, #8b8c8e 20%, #f4f4f4); border-radius: 25px; }";
  html += "label div:after { content: \"\"; position: absolute; display: block; height: 46px; width: 116px; left: 2px; top: 2px; border-radius: 23px; background: #828080; box-shadow: inset 0 0 30px 0 rgba(0,0,0,0.8); transition: .20s; }";
  html += "label i { display: block; width: 60px; height: 60px; position: absolute; background: linear-gradient(to top, #9e9e9e 20%, #f4f4f4); border-radius: 50%; box-shadow: 0 5px 10px 0 rgba(0,0,0,0.7); top: 10px; left: 15px; transition: .25s; }";
  html += "label i:after { content: \"\"; position: absolute; display: block; width: 52px; height: 52px; left: 4px; top: 4px; border-radius: 50%; background: #d5d4d4; z-index: 1; }";
  html += "label input:checked ~ i { top: 10px; left: 86px; }";
  html += "label input:checked + div:after { background: #f7931e; box-shadow: inset 0 0 30px 0 rgba(0,0,0,0.6); }";
  html += "label input:checked + div > .off { color: transparent; text-shadow: 0 1px 0 rgba(255,255,255,0); }";
  html += "label input:checked + div > .on { color: #c6631d; text-shadow: 0 1px 0 rgba(255,255,255, 0.3); }";
  html += "label:after { content: \"\"; position: absolute; display: block; width: 164px; height: 84px; border-radius: 42px; top: -2px; left: -2px; z-index: -1; background: linear-gradient(to bottom, #969494, #fff); }";
  html += "label:hover { cursor: pointer; }";
  html += "label:focus, label:active { outline: 0; }";
  html += ".on, .off { text-transform: uppercase; position: absolute; left: 17px; top: 50%; transform: translateY(-50%); font-size: 1.2em; font-weight: 600; z-index: 2; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; letter-spacing: 1px; transition: .25s; }";
  html += ".on { color: transparent; text-shadow: 0 1px 0 rgba(255,255,255,0); }";
  html += ".off { left: initial; right: 17px; color: #444; text-shadow: 0 1px 0 rgba(255,255,255, 0.2); }";
  html += ".warning {";
  html += "background-color: red;";
  html += "color: white;";
  html += "font-size: 1.2em;"; 
  html += "padding: 5px;"; 
  html += "position: fixed;";
  html += "top: 10px;";
  html += "left: 50%;";
  html += "transform: translateX(-50%);";
  html += "z-index: 1000;";
  html += "border-radius: 5px;";
  html += "display: none;";
  html += "}";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div>";
  html += "<label>";
  html += "<input type=\"checkbox\" onchange=\"toggleSystem(this)\" id=\"systemSwitch\"/>";
  html += "<div>";
  html += "<span class=\"on\">ON</span>";
  html += "<span class=\"off\">OFF</span>";
  html += "</div>";
  html += "<i></i>";
  html += "</label>";
  html += "</div>";
  html += "<div id=\"warningMessage\" class=\"warning\"><center>PINTU TERBUKA - BAHAYA!</center></div>";
  html += "<script>";
  html += "function toggleSystem(checkbox) {";
  html += "if (checkbox.checked) {";
  html += "fetch('/on');";
  html += "} else {";
  html += "fetch('/off');";
  html += "}";
  html += "}";
  html += "setInterval(() => {";
  html += "fetch('/status').then(response => response.text()).then(data => {";
  html += "document.getElementById('warningMessage').style.display = data === 'BUZZER_ON' ? 'block' : 'none';";
  html += "});";
  html += "}, 1000);"; // Periksa status buzzer setiap 1 detik
  html += "</script>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

void handleOn() {
  systemArmed = true;
  digitalWrite(lampPin, HIGH); // Nyalakan lampu
  mqttClient.publish("home/antimaling", "ON");
  server.send(200, "text/plain", "System ON");
}

void handleOff() {
  systemArmed = false;
  digitalWrite(lampPin, LOW);  // Matikan lampu
  digitalWrite(buzzerPin, LOW); // Matikan buzzer
  buzzerOn = false;
  mqttClient.publish("home/antimaling", "OFF");
  server.send(200, "text/plain", "System OFF");
}

void handleStatus() {
  if (buzzerOn) {
    server.send(200, "text/plain", "BUZZER_ON");
  } else {
    server.send(200, "text/plain", "BUZZER_OFF");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(lampPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP);

  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/status", handleStatus);
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  server.handleClient();

  // Magnetic switch logic with debounce
  int switchState = digitalRead(switchPin);
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (systemArmed && switchState == HIGH) {
      digitalWrite(buzzerPin, HIGH); // Aktifkan buzzer saat sistem aktif dan switch terpicu
      buzzerOn = true;
    } else {
      digitalWrite(buzzerPin, LOW);  // Nonaktifkan buzzer jika tidak terpicu
      buzzerOn = false;
    }
    lastDebounceTime = millis();
  }
}
