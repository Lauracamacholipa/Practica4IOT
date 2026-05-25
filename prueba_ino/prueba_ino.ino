#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ======================================================
// WIFI
// ======================================================

const char* WIFI_SSID = "UCB-PREMIUM";
const char* WIFI_PASS = "lacatoucb";

// ======================================================
// AWS IOT
// ======================================================

const char* MQTT_BROKER =
"a2clru565w1hxm-ats.iot.us-east-1.amazonaws.com";

const int MQTT_PORT = 8883;
const char* CLIENT_ID = "ESP-32";

// ======================================================
// CERTIFICADOS AWS
// PEGA LOS TUYOS AQUÍ
// ======================================================

const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUf0iyyXGbOwbJ8CwnIC9JvDGoJ/gwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDQyOTIyMjA0
MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOKqS1GFK+EV9fM7JvaO
BxkEkFH3hWZtvw5OJ5hQYwF0evaGbJSZ59EPMT1O+2i5Em3KxzKzFJ0rWOD6vMau
QYoQRq/QVgnVXbNqGRn652h6mT/Hk5L2gTVd9l97TbttAgsdI8lCJSN/jgtZF3bF
VC83XSurwWVtFr+9lW5JLbBRJVtRRwEisQl/4BJEMjRPGcRYtUaBaBdGN6nOqGEW
mjf8q7v2cw7dSXs5lWFEvVOLl21QyZ6Ifh01AEBlz+7FGxClwl62EP6H8nvpNdAd
LcrAhUnCBSmRI29tiENDpRDi2nW8QD5llyf5TIBt11Ha5Iaz0QDngIIfnhF/xTx7
YDsCAwEAAaNgMF4wHwYDVR0jBBgwFoAUCnrjgUB3OT5OhL2CZWFAwiwnUs0wHQYD
VR0OBBYEFKg0Lcm9Z0faM1W48/9fSmcWI9yzMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQB9Mr4ktl2C+ZJef5329MqX3J4G
0lhu+fagTeULZD2Wum0Bsc2osXDjnG7R66wIKuCHzROH67nZOfKkf6tgpfAws92L
931uzhszNinXFaAnRzgMfEQDF6qsLS2T2uTnwxDVP6AYWh0MHdt9TqYGAidLY0Is
Iyc9M2463duKjZl+RAj+xxXkjo1aEfnvhq4P05Y6fplWATACdHOs34KFt5u/3K59
EDaArO0vT4kAZCcKbE8p0jrNlZM3o7XOaJa4gFMp7Xl0yJ08Ob0zsJlANybogg0l
cC5lSRkq2C4xVDJJG6rAWczeYMSBaA6FbGUbNsg3yeuTknqjKx0phLVIa5iK
-----END CERTIFICATE-----
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA4qpLUYUr4RX18zsm9o4HGQSQUfeFZm2/Dk4nmFBjAXR69oZs
lJnn0Q8xPU77aLkSbcrHMrMUnStY4Pq8xq5BihBGr9BWCdVds2oZGfrnaHqZP8eT
kvaBNV32X3tNu20CCx0jyUIlI3+OC1kXdsVULzddK6vBZW0Wv72VbkktsFElW1FH
ASKxCX/gEkQyNE8ZxFi1RoFoF0Y3qc6oYRaaN/yru/ZzDt1JezmVYUS9U4uXbVDJ
noh+HTUAQGXP7sUbEKXCXrYQ/ofye+k10B0tysCFScIFKZEjb22IQ0OlEOLadbxA
PmWXJ/lMgG3XUdrkhrPRAOeAgh+eEX/FPHtgOwIDAQABAoIBAQCF7iG69XlL8nH1
WopXPOyst6mfBSZps4ckqMF0mmslvPG7IQqfc6EqQlPR6g1bf5+ICD1Tk8UmaSqH
JSDONMC9F4js8akk/lHzZUmXtE1vyqEqsn5cZBNMuUyOdokjqfF/J6SxksfnBcOk
qOvYXhi0HX6MA1yu1ZqPiTH2jH7Wor7x6fM3EgzStJALVHnIZ8kjlrh5VX9VhYCT
5V5bt5g1Q2DL8Y2vk6CTmTOHv3BkcifRKBXsq6xwgPcmuGm7TzEpdAii6KWJd/gF
/GalM1iESvY6pfsu3F+Ur+WJWfXrguCj+bqBkqKgUyLMmjBvNxMT+EMOUF0/cDPa
Ht+V9lAhAoGBAPzogVeMbiKkIAEx1EwIibaXHbeOiHBGY8bP5P/msSrMgMu9VSU4
X31c8aCVHCKQu+V3o7JwzmUkRos11lA2mADkdVyZaX97FnwfQ7SFhxyKwrcoQWkM
YKAilKT+bkqyUGlGdTpBQdbs/ONLDBtVC2dAgbbP+TDy6RPR/Qvv80ZHAoGBAOVv
qNgnAXsxZ1zGcI2GwL/kU5VK7shovosFZkWLrWpB+GJQHRNWVlexRN48IJ4evRqF
oq2IGbXIDfQoT/p/qQN60wL1SSC3vRS+7zL0eQwmFIessyhkOEVqTwyeS5ZBG9C7
mQIovM4Kc06U46vLeFe7DdrJ9qt6f4dnPL4ve+xtAoGBALu9tVZSTue0QnXgkaIn
rGM45rx+OFZwSl0psussS1RkHzAfSneo02nydwQn7IzUOEdter1Z9D0soplwy7kb
7darwVuOHrydmOFnLBF0FgpgD497eKMMpyX4xbN9lE4q9HpR7W/jRYbQwiHQchpQ
FXj+b8xPssJdcBxgg6I667K9AoGBALUabWmpaGbouVEnJmLgcjdGMJ/QlSlGXNl5
up+8wHb2eeyIllrUNLVlrRnK1mhZlSVhROa7Xqf5sOIADBmMOUoQ4o0cNswHnM3K
ZR+QkBgB2wS5UnXYnf5VCu7H6JxC6oTZ/84Pf/EXmKr3pbrOkkCV2zrPXtE+kXm9
CNqKusQFAoGAF+DxA1cYve2ViMIDV0RO2tSfZPWAK0ZL0aEUeOa7TC2GMrGBgIev
HMh8ssIPavzvgNeHLk4Icb1/SuL/uU7WLGpEPoh5j5eOqvaOHdpjGImlMZEWwuOm
1ZDiDXBDkYsuVYgfgOXTiQZ25ZIQoLuCBMvA4QsSGds8d156sOBEQxY=
-----END RSA PRIVATE KEY-----
)KEY";

// ======================================================
// SHADOW TOPICS
// ======================================================

const char* SHADOW_UPDATE_TOPIC =
"$aws/things/thing_test/shadow/update";

const char* SHADOW_DELTA_TOPIC =
"$aws/things/thing_test/shadow/update/delta";

const char* SHADOW_GET_TOPIC =
"$aws/things/thing_test/shadow/get";

const char* SHADOW_GET_ACCEPTED_TOPIC =
"$aws/things/thing_test/shadow/get/accepted";

// ======================================================
// PINES
// ======================================================

#define SENSOR_HUMEDAD 15
#define IN1_MOTOR 32
#define IN2_MOTOR 33

// ======================================================
// CALIBRACION SENSOR
// ======================================================

const int SENSOR_SECO = 2300;
const int SENSOR_MOJADO = 800;

// ======================================================
// VARIABLES SISTEMA
// ======================================================

bool irrigationOn = false;
String currentMode = "automatic";

int humidity = 0;

int thresholdLow = 30;
int thresholdHigh = 70;

unsigned long previousMillis = 0;
unsigned long interval = 1000;

// ======================================================
// MQTT
// ======================================================

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

StaticJsonDocument<512> jsonDoc;

// ======================================================
// FUNCIONES BOMBA
// ======================================================

void turnPumpOn() {

  digitalWrite(IN1_MOTOR, HIGH);
  digitalWrite(IN2_MOTOR, LOW);

  irrigationOn = true;

  Serial.println("BOMBA ENCENDIDA");
}

void turnPumpOff() {

  digitalWrite(IN1_MOTOR, LOW);
  digitalWrite(IN2_MOTOR, LOW);

  irrigationOn = false;

  Serial.println("BOMBA APAGADA");
}

// ======================================================
// SENSOR HUMEDAD
// ======================================================

int readHumidity() {

  int rawValue =
      analogRead(SENSOR_HUMEDAD);

  int humidityPercent = map(
      rawValue,
      SENSOR_SECO,
      SENSOR_MOJADO,
      0,
      100);

  humidityPercent =
      constrain(
          humidityPercent,
          0,
          100);

  Serial.println(
      "--------------------");

  Serial.print("ADC: ");
  Serial.println(rawValue);

  Serial.print("Humedad: ");
  Serial.print(humidityPercent);
  Serial.println("%");

  return humidityPercent;
}

// ======================================================
// REPORTAR SHADOW
// ======================================================

void updateShadow() {

  StaticJsonDocument<256> shadow;

  shadow["state"]["reported"]["mode"] =
      currentMode;

  shadow["state"]["reported"]["irrigation"] =
      irrigationOn;

  shadow["state"]["reported"]["humidity"] =
      humidity;

  shadow["state"]["reported"]["interval"] =
      interval / 1000;

  shadow["state"]["reported"]["thresholdLow"] =
      thresholdLow;

  shadow["state"]["reported"]["thresholdHigh"] =
      thresholdHigh;

  char buffer[512];

  serializeJson(shadow, buffer);

  client.publish(
      SHADOW_UPDATE_TOPIC,
      buffer);

  Serial.println(
      "Shadow actualizado");
}

// ======================================================
// CALLBACK MQTT
// ======================================================

void callback(
    char* topic,
    byte* payload,
    unsigned int length) {

  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("Mensaje recibido:");
  Serial.println(message);

  DeserializationError error =
      deserializeJson(jsonDoc, message);

  if (error) {

    Serial.println("Error JSON");
    return;
  }

  String topicStr = String(topic);

  JsonObject state;

  // ==========================================
  // DELTA TOPIC
  // state.mode
  // ==========================================
  if (topicStr == SHADOW_DELTA_TOPIC) {

    state = jsonDoc["state"];
  }

  // ==========================================
  // GET ACCEPTED
  // state.desired.mode
  // ==========================================
  else if (
      topicStr ==
      SHADOW_GET_ACCEPTED_TOPIC) {

    state =
        jsonDoc["state"]
               ["desired"];
  }

  else {

    return;
  }

  // ==========================================
  // MODE
  // ==========================================

  if (state.containsKey("mode")) {

    currentMode =
        state["mode"]
            .as<String>();

    Serial.print("Modo recibido: ");
    Serial.println(currentMode);
  }

  // ==========================================
  // THRESHOLDS
  // ==========================================

  if (
      state.containsKey(
          "thresholdLow")) {

    thresholdLow =
        state["thresholdLow"];
  }

  if (
      state.containsKey(
          "thresholdHigh")) {

    thresholdHigh =
        state["thresholdHigh"];
  }

  // ==========================================
  // MANUAL MODE
  // ==========================================

  if (
      currentMode ==
      "manual") {

    if (
        state.containsKey(
            "irrigation")) {

      bool irrigation =
          state["irrigation"];

      Serial.print(
          "Irrigation: ");

      Serial.println(
          irrigation);

      if (irrigation) {

        turnPumpOn();

      } else {

        turnPumpOff();
      }
    }
  }

  updateShadow();
}

// ======================================================
// WIFI
// ======================================================

void setupWiFi() {

  Serial.print(
      "Conectando WiFi");

  WiFi.begin(
      WIFI_SSID,
      WIFI_PASS);

  while (
      WiFi.status() !=
      WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(
      "WiFi conectado");

  Serial.println(
      WiFi.localIP());
}

// ======================================================
// MQTT
// ======================================================

void reconnectMQTT() {

  while (
      !client.connected()) {

    Serial.println(
        "Conectando MQTT...");

    if (
        client.connect(
            CLIENT_ID)) {

      Serial.println(
          "MQTT conectado");

      client.subscribe(
          SHADOW_DELTA_TOPIC);

      client.subscribe(
          SHADOW_GET_ACCEPTED_TOPIC);

      // pedir shadow actual
      client.publish(
          SHADOW_GET_TOPIC,
          "{}");

      Serial.println(
          "Shadow solicitado");

    } else {

      Serial.print(
          "Error MQTT: ");

      Serial.println(
          client.state());

      delay(5000);
    }
  }
}

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  pinMode(
      IN1_MOTOR,
      OUTPUT);

  pinMode(
      IN2_MOTOR,
      OUTPUT);

  pinMode(
      SENSOR_HUMEDAD,
      INPUT);

  analogReadResolution(12);

  turnPumpOff();

  setupWiFi();

  wifiClient.setCACert(
      AMAZON_ROOT_CA1);

  wifiClient
      .setCertificate(
          CERTIFICATE);

  wifiClient
      .setPrivateKey(
          PRIVATE_KEY);

  client.setServer(
      MQTT_BROKER,
      MQTT_PORT);

  client.setCallback(
      callback);

  Serial.println(
      "Sistema iniciado");
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  if (
      !client.connected()) {

    reconnectMQTT();
  }

  client.loop();

  unsigned long currentMillis =
      millis();

  if (
      currentMillis -
          previousMillis >=
      interval) {

    previousMillis =
        currentMillis;

    humidity =
        readHumidity();

    // ======================
    // AUTOMATIC MODE
    // ======================

    if (
        currentMode ==
        "automatic") {

      if (
          humidity <=
          thresholdLow) {

        if (
            !irrigationOn) {

          Serial.println(
              "Tierra seca");

          turnPumpOn();
        }

      } else if (
          humidity >=
          thresholdHigh) {

        if (
            irrigationOn) {

          Serial.println(
              "Tierra humeda");

          turnPumpOff();
        }

      } else {

        Serial.println(
            "Mantener estado");
      }
    }

    updateShadow();

    Serial.print(
        "Estado bomba: ");

    Serial.println(
        irrigationOn
            ? "ENCENDIDA"
            : "APAGADA");
  }
}