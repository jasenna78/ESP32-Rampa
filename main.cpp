#include <Arduino.h>
#include <WiFi.h>
#include "cJSON.h"
#include <espMqttClient.h>
#include "Button.h"
#include "Ramp.h"

#define WIFI_SSID "iPhone od: Jasena"
#define WIFI_PASSWORD "jasena78"

bool parse_mqtt_message(char *pyload);

espMqttClient mqttClient;
bool reconnectMqtt = false;
uint32_t lastReconnect = 0;

void connectToWiFi()
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt()
{
  Serial.println("Connecting to MQTT...");
  if (!mqttClient.connect())
  {
    reconnectMqtt = true;
    lastReconnect = millis();
    Serial.println("Connecting failed.");
  }
  else
  {
    reconnectMqtt = false;
  }
}

void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    connectToMqtt();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WiFi lost connection");
    break;
  default:
    break;
  }
}

void onMqttConnect(bool sessionPresent)
{
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("jasena/rampa/odAplikacije", 0);
  Serial.print("Subscribing at QoS 0, packetId: ");
  Serial.println(packetIdSub);
  mqttClient.publish("foo/bar", 0, true, "test 1");
  Serial.println("Publishing at QoS 0");
  uint16_t packetIdPub1 = mqttClient.publish("foo/bar", 1, true, "test 2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
  uint16_t packetIdPub2 = mqttClient.publish("foo/bar", 2, true, "test 3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);
}

void onMqttDisconnect(espMqttClientTypes::DisconnectReason reason)
{
  Serial.printf("Disconnected from MQTT: %u.\n", static_cast<uint8_t>(reason));

  if (WiFi.isConnected())
  {
    reconnectMqtt = true;
    lastReconnect = millis();
  }
}

void onMqttSubscribe(uint16_t packetId, const espMqttClientTypes::SubscribeReturncode *codes, size_t len)
{
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  for (size_t i = 0; i < len; ++i)
  {
    Serial.print("  qos: ");
    Serial.println(static_cast<uint8_t>(codes[i]));
  }
}

void onMqttUnsubscribe(uint16_t packetId)
{
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(const espMqttClientTypes::MessageProperties &properties, const char *topic, const uint8_t *payload, size_t len, size_t index, size_t total)
{

  char buffer[100];

  for (int i = 0; i < len; ++i)
  {
    buffer[i] = payload[i];
  }
  
  buffer[len] = '\0';

  parse_mqtt_message(buffer);
}

void onMqttPublish(uint16_t packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void mqttSetup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println();

  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);
  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer("mqtt.eclipseprojects.io", 1883);

  connectToWiFi();
}

//******************************************************************************

void carPassed();
void carEntered();
void onClick_dugme1();

Button dugme(6, 5, &onClick_dugme1);

Ramp rampa(4, 3, 11, &carEntered, &carPassed);

void carEntered()
{

  static int counter1 = 0;
  counter1++;
  Serial.print("Auto je doslo ");
  Serial.print(counter1);
  Serial.println(" puta");
}

void carPassed()
{

  static int counter2 = 0;
  counter2++;
  Serial.print("Auto je proslo na rampi ");
  Serial.print(counter2);
  Serial.println(" puta");
  rampa.close_ramp();
}

void onClick_dugme1()
{
  static int counter = 0;
  Serial.print("Klik je detektovan ");
  Serial.print(counter);
  Serial.println(" puta");
  counter++;
  if (!rampa.get_car_on_rump())
  {
    rampa.open_ramp();
  }
}

void setup()
{

  pinMode(7, INPUT);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  mqttSetup();
}

void loop()
{

  dugme.run();

  static uint32_t last_ch = millis();
  uint32_t ellapsed = millis() - last_ch;
  if (ellapsed > 100)
  {
    rampa.run();
    last_ch = millis();
  }
}

bool parse_mqtt_message(char *pyload)
{
  bool return_state = false;
  cJSON *root = cJSON_Parse(pyload);

  if (cJSON_HasObjectItem(root, "open"))
  {
    int stanjeManipulacije = cJSON_GetObjectItem(root, "open")->valueint;
    if (stanjeManipulacije == 1)
    {

      if (!rampa.get_car_on_rump())
      {
        rampa.open_ramp();
      }

      return_state = true;
    }
    else if (stanjeManipulacije == 0)
    {

      if (rampa.get_car_on_rump())
      {

        rampa.close_ramp();
      }

      return_state = true;
    }
    else
    {
      Serial.println("Nepoznato stanje manipulacije");
    }
  }

  if (cJSON_HasObjectItem(root, "printState"))
  {
    if (rampa.get_ramp_state() == stanjeRampe::open_ramp_t)
    {
      Serial.println("Rampa je podignuta");
    }
    else
    {
      Serial.println("Rampa je spustena");
    }
  }

  cJSON_Delete(root);
  return return_state;
}
