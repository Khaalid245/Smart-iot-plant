#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "secrets.h"

// ==================================================
// WIFI
// ==================================================

const char* ssid = SECRET_WIFI_SSID;
const char* password = SECRET_WIFI_PASSWORD;

// ==================================================
// THINGSPEAK
// ==================================================

const char* apiKey = SECRET_THINGSPEAK_API_KEY;
const char* server = "http://api.thingspeak.com/update";

// ==================================================
// MQTT - HIVE MQ CLOUD
// ==================================================

const char* mqttHost = SECRET_MQTT_HOST;

const int mqttPort = SECRET_MQTT_PORT;

const char* mqttUsername = SECRET_MQTT_USERNAME;

// IMPORTANT:
// Put your HiveMQ password here on your computer.
// Do NOT share the password in chat.
const char* mqttPassword = SECRET_MQTT_PASSWORD;

const unsigned long mqttPublishInterval = 20000;
const unsigned long mqttReconnectInterval = 5000;

WiFiClientSecure mqttTLSClient;
PubSubClient mqttClient(mqttTLSClient);

unsigned long lastMQTTPublish = 0;
unsigned long lastMQTTReconnectAttempt = 0;

char mqttClientId[40];

// Let's Encrypt ISRG Root X1.
// HiveMQ Cloud Serverless uses a Let's Encrypt certificate chain.
const char* mqttRootCA = R"CERT(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)CERT";

// ==================================================
// MQTT TOPICS
// ==================================================

const char* MQTT_TEMP_TOPIC =
  "smartfarm/esp32-01/sensors/temperature";

const char* MQTT_HUMIDITY_TOPIC =
  "smartfarm/esp32-01/sensors/humidity";

const char* MQTT_SOIL_TOPIC =
  "smartfarm/esp32-01/sensors/soil";

const char* MQTT_LIGHT_TOPIC =
  "smartfarm/esp32-01/sensors/light";

const char* MQTT_PUMP_TOPIC =
  "smartfarm/esp32-01/status/pump";

const char* MQTT_FAN_TOPIC =
  "smartfarm/esp32-01/status/fan";

const char* MQTT_LEDS_TOPIC =
  "smartfarm/esp32-01/status/leds";

// ==================================================
// WEB SERVER
// ==================================================

WebServer webServer(80);

// ==================================================
// SENSOR PINS
// ==================================================

#define DHT_PIN   18
#define SOIL_PIN  34
#define LDR_PIN   35

// ==================================================
// DARKNESS LEDS - ADDED ONLY
// ==================================================

#define LED1_PIN 12
#define LED2_PIN 13

// Light percentage is already calculated from the original LDR.
// We use a RANGE instead of one fixed ON/OFF point.
//
// Low lightPercent = dark
// High lightPercent = bright
//
// The gap between these two values prevents the LEDs from
// rapidly switching when sunlight changes or clouds pass.
const int LIGHT_DARK_ON  = 20;
const int LIGHT_BRIGHT_OFF = 30;

// Small averaging filter for a more stable LDR reading.
const int LDR_SAMPLES = 10;

// ==================================================
// OLED
// ==================================================

#define OLED_SDA 21
#define OLED_SCL 22

// ==================================================
// L298N
// ==================================================

// FAN -> OUT1 / OUT2
#define FAN_IN1 33
#define FAN_IN2 25

// PUMP -> OUT3 / OUT4
#define PUMP_IN3 26
#define PUMP_IN4 27

// ==================================================
// DHT22
// ==================================================

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// ==================================================
// OLED SH1106
// ==================================================

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// ==================================================
// TIMERS
// ==================================================

unsigned long lastSensorRead = 0;
unsigned long lastThingSpeak = 0;

const unsigned long sensorInterval = 2000;
const unsigned long thingSpeakInterval = 20000;

// ==================================================
// SENSOR VALUES
// ==================================================

float temperature = 0;
float humidity = 0;

int soilRaw = 0;
int soilPercent = 0;

int ldrRaw = 0;
int lightPercent = 0;

bool dhtOK = false;

// ==================================================
// SOIL CALIBRATION
// ==================================================

// ==================================================
// SOIL MOISTURE / LEVEL CALIBRATION
// ==================================================
// Continuous 0-100% range based on the actual probe reading.
// Probe outside water = 3840 -> 0%
// Probe fully in water = 1669 -> 100%
//
// There are no fixed 25%, 50%, or 75% levels.
// Values between the two calibration points are calculated
// continuously from the analog sensor reading.
const int SOIL_DRY_VALUE = 3840;
const int SOIL_WET_VALUE = 1669;

// ==================================================
// PUMP LOGIC
// ==================================================

const int PUMP_ON_SOIL = 60;
const int PUMP_OFF_SOIL = 100;

// ==================================================
// FAN LOGIC
// ==================================================

const float FAN_ON_TEMP = 28.0;

// ==================================================
// MOTOR STATES
// ==================================================

bool pumpState = false;
bool fanState = false;

// ==================================================
// FAN MODE
// ==================================================

// true  = AUTO
// false = MANUAL

bool fanAutoMode = true;

// ==================================================
// DARKNESS LED STATE - ADDED ONLY
// ==================================================

bool darknessLEDState = false;
int filteredLdrRaw = 0;

// ==================================================
// FUNCTION DECLARATIONS
// ==================================================

void readSensors();
void updateMotorControl();
void updateOLED();
void sendToThingSpeak();

void pumpON();
void pumpOFF();

void fanON();
void fanOFF();

void handleRoot();
void handleData();

void handleFanAuto();
void handleFanOn();
void handleFanOff();

void updateDarknessLEDs();

void syncTimeForMQTT();
void setupMQTT();
void reconnectMQTT();
void publishMQTTData();

// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);

  delay(500);

  // ==================================================
  // MOTOR PINS
  // ==================================================

  pinMode(FAN_IN1, OUTPUT);
  pinMode(FAN_IN2, OUTPUT);

  pinMode(PUMP_IN3, OUTPUT);
  pinMode(PUMP_IN4, OUTPUT);

  // Motors OFF

  fanOFF();
  pumpOFF();

  // ==================================================
  // DARKNESS LEDS - ADDED ONLY
  // ==================================================

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  // ==================================================
  // DHT
  // ==================================================

  dht.begin();

  // ==================================================
  // ADC
  // ==================================================

  analogReadResolution(12);

  // ==================================================
  // OLED
  // ==================================================

  Wire.begin(OLED_SDA, OLED_SCL);

  oled.begin();

  oled.clearBuffer();

  oled.setFont(u8g2_font_ncenB08_tr);

  oled.drawStr(20, 25, "ESP32");
  oled.drawStr(10, 45, "Smart Plant");

  oled.sendBuffer();

  delay(1500);

  // ==================================================
  // WIFI
  // ==================================================

  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP32 SMART PLANT");
  Serial.println("==============================");

  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  int wifiAttempts = 0;

  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 40) {

    delay(500);

    Serial.print(".");

    wifiAttempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi connected!");

    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.println();
    Serial.println("Open this address on your phone:");
    Serial.print("http://");
    Serial.println(WiFi.localIP());

  }

  else {

    Serial.println("WiFi connection FAILED!");

  }

  // ==================================================
  // MQTT TIME SYNC FOR TLS
  // ==================================================

  if (WiFi.status() == WL_CONNECTED) {
    syncTimeForMQTT();
    setupMQTT();
  }

  // ==================================================
  // WEB SERVER
  // ==================================================

  webServer.on("/", handleRoot);

  webServer.on("/data", handleData);

  webServer.on("/fan/auto", handleFanAuto);

  webServer.on("/fan/on", handleFanOn);

  webServer.on("/fan/off", handleFanOff);

  webServer.begin();

  Serial.println();
  Serial.println("Web server started!");

  // ==================================================
  // INITIAL SENSOR READING
  // ==================================================

  readSensors();

  updateMotorControl();

  updateDarknessLEDs();

  updateOLED();

  lastSensorRead = millis();

  lastThingSpeak = millis();
}

// ==================================================
// LOOP
// ==================================================

void loop() {

  unsigned long currentTime = millis();

  // ==================================================
  // WEB SERVER
  // ==================================================

  webServer.handleClient();

  // ==================================================
  // MQTT
  // ==================================================

  if (WiFi.status() == WL_CONNECTED) {

    if (!mqttClient.connected()) {
      reconnectMQTT();
    }

    mqttClient.loop();

    if (mqttClient.connected() &&
        currentTime - lastMQTTPublish >= mqttPublishInterval) {

      lastMQTTPublish = currentTime;
      publishMQTTData();
    }
  }

  // ==================================================
  // SENSOR READING
  // ==================================================

  if (currentTime - lastSensorRead >= sensorInterval) {

    lastSensorRead = currentTime;

    readSensors();

    updateMotorControl();

    updateDarknessLEDs();

    updateOLED();
  }

  // ==================================================
  // THINGSPEAK
  // ==================================================

  if (currentTime - lastThingSpeak >= thingSpeakInterval) {

    lastThingSpeak = currentTime;

    sendToThingSpeak();
  }
}

// ==================================================
// MQTT TIME SYNC FOR TLS
// ==================================================

void syncTimeForMQTT() {

  Serial.println();
  Serial.println("Syncing time for MQTT TLS...");

  configTime(
    0,
    0,
    "pool.ntp.org",
    "time.nist.gov"
  );

  struct tm timeinfo;

  for (int i = 0; i < 20; i++) {

    if (getLocalTime(&timeinfo, 500)) {

      Serial.println("Time synchronized.");

      return;
    }

    delay(250);
  }

  Serial.println("WARNING: Time synchronization failed.");
}

// ==================================================
// MQTT SETUP
// ==================================================

void setupMQTT() {

  mqttTLSClient.setInsecure();

  mqttClient.setServer(
    mqttHost,
    mqttPort
  );

  mqttClient.setKeepAlive(30);
  mqttClient.setSocketTimeout(3);
  mqttClient.setBufferSize(512);

  uint64_t chipId = ESP.getEfuseMac();

  snprintf(
    mqttClientId,
    sizeof(mqttClientId),
    "smartfarm-%04X%08X",
    (uint16_t)(chipId >> 32),
    (uint32_t)chipId
  );

  Serial.println();
  Serial.println("MQTT configured.");
  Serial.print("MQTT Host: ");
  Serial.println(mqttHost);
  Serial.print("MQTT Client ID: ");
  Serial.println(mqttClientId);
}

// ==================================================
// MQTT RECONNECT
// ==================================================

void reconnectMQTT() {

  if (millis() - lastMQTTReconnectAttempt <
      mqttReconnectInterval) {

    return;
  }

  lastMQTTReconnectAttempt = millis();

  if (mqttPassword == nullptr ||
      strcmp(mqttPassword, "YOUR_HIVEMQ_PASSWORD") == 0) {

    Serial.println();
    Serial.println("MQTT password not configured.");
    return;
  }

  Serial.println();
  Serial.println("Connecting to HiveMQ Cloud...");

  if (mqttClient.connect(
        mqttClientId,
        mqttUsername,
        mqttPassword)) {

    Serial.println("MQTT connected!");

    // Publish an online status message.
    mqttClient.publish(
      "smartfarm/esp32-01/status/connection",
      "ONLINE",
      true
    );

    lastMQTTPublish = 0;
  }

  else {

    Serial.print("MQTT connection failed. State: ");
    Serial.println(mqttClient.state());
  }
}

// ==================================================
// MQTT PUBLISH SENSOR + DEVICE STATUS
// ==================================================

void publishMQTTData() {

  if (!mqttClient.connected()) {
    Serial.println("MQTT publish skipped: client not connected.");
    return;
  }

  char value[32];
  bool allPublished = true;

  snprintf(value, sizeof(value), "%.2f", temperature);
  bool tempOK = mqttClient.publish(MQTT_TEMP_TOPIC, value, true);
  Serial.print("Temperature publish: ");
  Serial.println(tempOK ? "SUCCESS" : "FAILED");
  if (!tempOK) allPublished = false;

  snprintf(value, sizeof(value), "%.2f", humidity);
  bool humidityOK = mqttClient.publish(MQTT_HUMIDITY_TOPIC, value, true);
  Serial.print("Humidity publish: ");
  Serial.println(humidityOK ? "SUCCESS" : "FAILED");
  if (!humidityOK) allPublished = false;

  snprintf(value, sizeof(value), "%d", soilPercent);
  bool soilOK = mqttClient.publish(MQTT_SOIL_TOPIC, value, true);
  Serial.print("Soil publish: ");
  Serial.println(soilOK ? "SUCCESS" : "FAILED");
  if (!soilOK) allPublished = false;

  snprintf(value, sizeof(value), "%d", lightPercent);
  bool lightOK = mqttClient.publish(MQTT_LIGHT_TOPIC, value, true);
  Serial.print("Light publish: ");
  Serial.println(lightOK ? "SUCCESS" : "FAILED");
  if (!lightOK) allPublished = false;

  bool pumpOK = mqttClient.publish(
    MQTT_PUMP_TOPIC,
    pumpState ? "ON" : "OFF",
    true
  );
  Serial.print("Pump publish: ");
  Serial.println(pumpOK ? "SUCCESS" : "FAILED");
  if (!pumpOK) allPublished = false;

  bool fanOK = mqttClient.publish(
    MQTT_FAN_TOPIC,
    fanState ? "ON" : "OFF",
    true
  );
  Serial.print("Fan publish: ");
  Serial.println(fanOK ? "SUCCESS" : "FAILED");
  if (!fanOK) allPublished = false;

  bool ledsOK = mqttClient.publish(
    MQTT_LEDS_TOPIC,
    darknessLEDState ? "ON" : "OFF",
    true
  );
  Serial.print("LEDs publish: ");
  Serial.println(ledsOK ? "SUCCESS" : "FAILED");
  if (!ledsOK) allPublished = false;

  Serial.print("MQTT publish result: ");
  Serial.println(allPublished ? "ALL SUCCESS" : "ONE OR MORE FAILED");
}

// ==================================================
// FAN ON
// ==================================================

void fanON() {

  fanState = true;

  digitalWrite(FAN_IN1, HIGH);
  digitalWrite(FAN_IN2, LOW);

}

// ==================================================
// FAN OFF
// ==================================================

void fanOFF() {

  fanState = false;

  digitalWrite(FAN_IN1, LOW);
  digitalWrite(FAN_IN2, LOW);

}

// ==================================================
// PUMP ON
// ==================================================

void pumpON() {

  pumpState = true;

  digitalWrite(PUMP_IN3, HIGH);
  digitalWrite(PUMP_IN4, LOW);

}

// ==================================================
// PUMP OFF
// ==================================================

void pumpOFF() {

  pumpState = false;

  digitalWrite(PUMP_IN3, LOW);
  digitalWrite(PUMP_IN4, LOW);

}

// ==================================================
// READ SENSORS
// ==================================================

void readSensors() {

  // ==================================================
  // DHT22
  // ==================================================

  float newTemperature = dht.readTemperature();

  float newHumidity = dht.readHumidity();

  if (!isnan(newTemperature) && !isnan(newHumidity)) {

    temperature = newTemperature;

    humidity = newHumidity;

    dhtOK = true;

  }

  else {

    dhtOK = false;

  }

  // ==================================================
  // SOIL
  // ==================================================

  soilRaw = analogRead(SOIL_PIN);

// Continuous 0-100% calculation.
// No fixed 25%, 50%, or 75% levels are used.
float soilValue =
  ((float)(SOIL_DRY_VALUE - soilRaw) /
   (float)(SOIL_DRY_VALUE - SOIL_WET_VALUE)) * 100.0;

soilPercent = (int)round(soilValue);

soilPercent = constrain(
  soilPercent,
  0,
  100
);

  // ==================================================
  // LDR
  // ==================================================

  // ==================================================
  // LDR - AVERAGED READING FOR STABILITY
  // ==================================================

  long ldrTotal = 0;

  for (int i = 0; i < LDR_SAMPLES; i++) {
    ldrTotal += analogRead(LDR_PIN);
    delay(2);
  }

  filteredLdrRaw = ldrTotal / LDR_SAMPLES;
  ldrRaw = filteredLdrRaw;

  lightPercent = map(
    ldrRaw,
    4095,
    0,
    0,
    100
  );

  lightPercent = constrain(
    lightPercent,
    0,
    100
  );

  // ==================================================
  // SERIAL
  // ==================================================

  Serial.println();
  Serial.println("------------------------------");

  if (dhtOK) {

    Serial.print("Temperature : ");
    Serial.print(temperature, 1);
    Serial.println(" C");

    Serial.print("Humidity    : ");
    Serial.print(humidity, 1);
    Serial.println(" %");

  }

  else {

    Serial.println("DHT22 ERROR");

  }

  Serial.print("Soil Raw    : ");
  Serial.println(soilRaw);

  Serial.print("Soil        : ");
  Serial.print(soilPercent);
  Serial.println(" %");

  Serial.print("LDR Raw     : ");
  Serial.println(ldrRaw);

  Serial.print("Light       : ");
  Serial.print(lightPercent);
  Serial.println(" %");

  Serial.print("Dark LEDs   : ");
  Serial.println(darknessLEDState ? "ON" : "OFF");

  Serial.print("Pump        : ");
  Serial.println(pumpState ? "ON" : "OFF");

  Serial.print("Fan         : ");
  Serial.println(fanState ? "ON" : "OFF");

  Serial.print("Fan Mode    : ");
  Serial.println(fanAutoMode ? "AUTO" : "MANUAL");

  Serial.println("------------------------------");
}

// ==================================================
// MOTOR CONTROL
// ==================================================

void updateMotorControl() {

  // ==================================================
  // PUMP - ALWAYS AUTOMATIC
  // ==================================================

  if (soilPercent <= PUMP_ON_SOIL) {

    pumpON();

  }

  else if (soilPercent >= PUMP_OFF_SOIL) {

    pumpOFF();

  }

  // ==================================================
  // FAN - ONLY AUTOMATIC IN AUTO MODE
  // ==================================================

  if (fanAutoMode && dhtOK) {

    if (temperature > FAN_ON_TEMP) {

      fanON();

    }

    else {

      fanOFF();

    }
  }
}

// ==================================================
// DARKNESS LED CONTROL - ADDED ONLY
// ==================================================

void updateDarknessLEDs() {

  // Very low light -> turn LEDs ON.
  if (lightPercent <= LIGHT_DARK_ON) {

    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);

    darknessLEDState = true;
  }

  // Clearly bright -> turn LEDs OFF.
  else if (lightPercent >= LIGHT_BRIGHT_OFF) {

    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);

    darknessLEDState = false;
  }

  // Between 20% and 30%:
  // keep the previous state.
  // This prevents flickering during changing sunlight.
}

// ==================================================
// OLED
// ==================================================

void updateOLED() {

  oled.clearBuffer();

  oled.setFont(u8g2_font_6x10_tf);

  // Temperature

  oled.drawStr(0, 10, "Temp:");

  if (dhtOK) {

    char text[20];

    sprintf(
      text,
      "%.1f C",
      temperature
    );

    oled.drawStr(45, 10, text);

  }

  else {

    oled.drawStr(45, 10, "ERROR");

  }

  // Humidity

  oled.drawStr(0, 23, "Humidity:");

  if (dhtOK) {

    char text[20];

    sprintf(
      text,
      "%.1f %%",
      humidity
    );

    oled.drawStr(55, 23, text);

  }

  else {

    oled.drawStr(55, 23, "ERROR");

  }

  // Soil

  oled.drawStr(0, 36, "Soil:");

  char soilText[20];

  sprintf(
    soilText,
    "%d %%",
    soilPercent
  );

  oled.drawStr(45, 36, soilText);

  // Light

  oled.drawStr(0, 49, "Light:");

  char lightText[20];

  sprintf(
    lightText,
    "%d %%",
    lightPercent
  );

  oled.drawStr(45, 49, lightText);

  // Pump

  oled.drawStr(0, 62, "P:");

  if (pumpState) {

    oled.drawStr(15, 62, "ON");

  }

  else {

    oled.drawStr(15, 62, "OFF");

  }

  // Fan

  oled.drawStr(55, 62, "F:");

  if (fanState) {

    oled.drawStr(70, 62, "ON");

  }

  else {

    oled.drawStr(70, 62, "OFF");

  }

  oled.sendBuffer();
}

// ==================================================
// WEB PAGE
// ==================================================

void handleRoot() {

  const char html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>ESP32 Smart Plant</title>

<style>

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 15px;
  font-family: Arial, sans-serif;
  background: #101010;
  color: white;
}

.container {
  max-width: 500px;
  margin: auto;
}

h1 {
  text-align: center;
  color: #00e676;
}

.card {
  background: #202020;
  border-radius: 15px;
  padding: 15px;
  margin-bottom: 12px;
  text-align: center;
}

.value {
  font-size: 30px;
  font-weight: bold;
}

.status {
  font-size: 20px;
  margin: 8px;
}

button {
  width: 30%;
  padding: 13px 5px;
  margin: 4px;
  border: none;
  border-radius: 10px;
  font-size: 15px;
  color: white;
}

.green {
  background: #00a844;
}

.red {
  background: #d00000;
}

.blue {
  background: #1769e0;
}

</style>

</head>

<body>

<div class="container">

<h1> Smart Plant</h1>

<div class="card">

<h2> Temperature</h2>

<div class="value">
<span id="temp">--</span> °C
</div>

</div>

<div class="card">

<h2> Humidity</h2>

<div class="value">
<span id="humidity">--</span> %
</div>

</div>

<div class="card">

<h2> Soil Moisture</h2>

<div class="value">
<span id="soil">--</span> %
</div>

</div>

<div class="card">

<h2> Light</h2>

<div class="value">
<span id="light">--</span> %
</div>

</div>

<div class="card">

<h2> Pump</h2>

<div class="status">
Pump: <b id="pump">--</b>
</div>

</div>

<div class="card">

<h2> Fan</h2>

<div class="status">
Fan: <b id="fan">--</b>
</div>

<div class="status">
Mode: <b id="mode">--</b>
</div>

<button class="green"
onclick="fanOn()">
ON
</button>

<button class="red"
onclick="fanOff()">
OFF
</button>

<button class="blue"
onclick="fanAuto()">
AUTO
</button>

</div>

<div class="card">

<p><b>Automatic Fan</b></p>

<p>Temperature < 28°C  Fan ON</p>

<p>Temperature > 28°C  Fan OFF</p>

<hr>

<p><b>Automatic Pump</b></p>

<p>Soil < 60%  Pump ON</p>

<p>Soil > 100%  Pump OFF</p>

</div>

</div>

<script>

function updateData() {

  fetch('/data')
  .then(response => response.json())
  .then(data => {

    document.getElementById('temp').innerText =
      data.temperature;

    document.getElementById('humidity').innerText =
      data.humidity;

    document.getElementById('soil').innerText =
      data.soil;

    document.getElementById('light').innerText =
      data.light;

    document.getElementById('pump').innerText =
      data.pump ? 'ON' : 'OFF';

    document.getElementById('fan').innerText =
      data.fan ? 'ON' : 'OFF';

    document.getElementById('mode').innerText =
      data.auto ? 'AUTO' : 'MANUAL';

  })

  .catch(error => {

    console.log('Connection error');

  });

}

function fanOn() {

  fetch('/fan/on');

}

function fanOff() {

  fetch('/fan/off');

}

function fanAuto() {

  fetch('/fan/auto');

}

updateData();

setInterval(updateData, 1000);

</script>

</body>

</html>

)rawliteral";

  webServer.send(
    200,
    "text/html",
    html
  );
}

// ==================================================
// JSON DATA
// ==================================================

void handleData() {

  String json;

  json.reserve(250);

  json = "{";

  json += "\"temperature\":";
  json += String(temperature, 1);

  json += ",\"humidity\":";
  json += String(humidity, 1);

  json += ",\"soil\":";
  json += String(soilPercent);

  json += ",\"light\":";
  json += String(lightPercent);

  json += ",\"pump\":";
  json += pumpState ? "true" : "false";

  json += ",\"fan\":";
  json += fanState ? "true" : "false";

  json += ",\"auto\":";
  json += fanAutoMode ? "true" : "false";

  json += "}";

  webServer.send(
    200,
    "application/json",
    json
  );
}

// ==================================================
// FAN AUTO
// ==================================================

void handleFanAuto() {

  fanAutoMode = true;

  updateMotorControl();

  webServer.send(
    200,
    "text/plain",
    "Fan AUTO mode"
  );
}

// ==================================================
// FAN MANUAL ON
// ==================================================

void handleFanOn() {

  fanAutoMode = false;

  fanON();

  Serial.println("WEB: FAN MANUAL ON");

  webServer.send(
    200,
    "text/plain",
    "Fan ON"
  );
}

// ==================================================
// FAN MANUAL OFF
// ==================================================

void handleFanOff() {

  fanAutoMode = false;

  fanOFF();

  Serial.println("WEB: FAN MANUAL OFF");

  webServer.send(
    200,
    "text/plain",
    "Fan OFF"
  );
}

// ==================================================
// THINGSPEAK
// ==================================================

void sendToThingSpeak() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi disconnected!");

    return;
  }

  if (!dhtOK) {

    Serial.println(
      "DHT22 invalid - ThingSpeak not updated."
    );

    return;
  }

  HTTPClient http;

  String url =
    String(server) +
    "?api_key=" + apiKey +
    "&field1=" + String(temperature, 2) +
    "&field2=" + String(humidity, 2) +
    "&field3=" + String(soilPercent) +
    "&field4=" + String(lightPercent);

  Serial.println();
  Serial.println("Sending data to ThingSpeak...");

  http.begin(url);

  http.setTimeout(3000);

  int httpCode = http.GET();

  if (httpCode > 0) {

    Serial.print("ThingSpeak HTTP Code: ");
    Serial.println(httpCode);

    String response = http.getString();

    Serial.print("Response: ");
    Serial.println(response);

  }

  else {

    Serial.print("ThingSpeak Error: ");

    Serial.println(
      http.errorToString(httpCode)
    );

  }

  http.end();
}
