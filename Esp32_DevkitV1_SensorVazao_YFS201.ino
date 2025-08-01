#include <WiFi.h>
#include <WebServer.h>

// === Replace with your Wi-Fi credentials ===
const char* ssid = "G75-MVC";
const char* password = "Valen666";

// === Pin definitions ===
#define LED_BUILTIN 2
#define SENSOR  27

// === Flow sensor variables ===
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

WebServer server(80);

// === Interrupt routine ===
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);

  // === Connect to Wi-Fi ===
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // === Web server endpoint ===
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>ESP32 Flow Sensor</title></head><body>";
    html += "<h1>ESP32 Flow Sensor Monitor</h1>";
    html += "<p><strong>Flow rate:</strong> " + String(flowRate, 2) + " L/min</p>";
    html += "<p><strong>Total Volume:</strong> " + String(totalMilliLitres) + " mL (" + String(totalMilliLitres / 1000.0, 2) + " L)</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;

    Serial.print("Flow rate: ");
    Serial.print(flowRate, 2);
    Serial.print(" L/min\t");

    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print(" mL / ");
    Serial.print(totalMilliLitres / 1000.0, 2);
    Serial.println(" L");
  }

  // Handle HTTP requests
  server.handleClient();
}
