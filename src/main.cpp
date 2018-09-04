/*
* @Author: Ramon Melo
* @Date:   2018-08-18
* @Last Modified by:   Ramon Melo
* @Last Modified time: 2018-08-18
*/

#include "mes_defines.h"

#include "TypeSelector.h"

#include <Ticker.h>
#include "Arduino.h"
#include <ArduinoJson.h>
#include <NTPtimeESP.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <queue>

struct Event
{
  char type;
  time_t time;
};

NTPtime NTPch("0.br.pool.ntp.org");
strDateTime currentDate;
time_t currentTime = 0;
time_t lastReconnectAttempt = 0;
time_t lastSend = 0;
time_t sendRate = 2;

WiFiClient espClient;
PubSubClient client(espClient);

Ticker mailman;

std::queue<Event> eventQueue;

mes::TypeSelector typeSelector(INPUT_1, INPUT_2);

time_t getNtpTime()
{
  Serial.println("Updating date...");

  do
  {
    currentDate = NTPch.getNTPtime(-3.0, 0);
    delay(100);
    Serial.print("-");
  } while (!currentDate.valid);

  if (currentDate.valid)
  {
    setSyncInterval(300);
    Serial.println("Valid date");

    return currentDate.epochTime;
  }

  return 0;
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.print(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void connect()
{
  Serial.println("Connecting to Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected");
}

void hibernate()
{
  Serial.println("Max sleep time:");
  Serial.print((unsigned long)ESP.deepSleepMax());

  Serial.println("bye");
  ESP.deepSleep(5e6);
}

void buildMessage(String *jsonStr, char type, time_t eventTime)
{
  const size_t bufferSize = JSON_OBJECT_SIZE(2);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject &root = jsonBuffer.createObject();
  root["p"] = type;
  root["t"] = eventTime;

  root.printTo(*jsonStr);
}

void sendEvent()
{
  if (!eventQueue.empty())
  {
    Event event = eventQueue.front();

    String msg;
    buildMessage(&msg, event.type, event.time);

    if (client.connected())
    {
      client.publish(MQTT_TOPIC, msg.c_str());
      Serial.print("_");

      eventQueue.pop();
    }
  }
}

void event()
{
  if ((currentTime - lastSend) > sendRate)
  {
    lastSend = currentTime;

    if (client.connected())
    {
      String msg;
      buildMessage(&msg, 'A', currentTime);

      client.publish(MQTT_TOPIC, msg.c_str());
      Serial.print(".");
    }
    else
    {
      Event event;
      event.type = 'A';
      event.time = lastSend;

      eventQueue.push(event);

      Serial.println("cached event");
    }
  }
}

boolean reconnect()
{
#ifdef MQTT_AUTH
  if (client.connect(MES_DEVICE_ID, MQTT_USER, MQTT_PASS))
#else
  if (client.connect(MES_DEVICE_ID))
#endif
  {
    // Once connected, publish an announcement...
    client.publish(MQTT_TOPIC, "I'm Alive!");
  }

  return client.connected();
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }

  Serial.println("Booted");

  connect();
  client.setServer(MQTT_BROKER, MQTT_PORT);

  pinMode(INPUT_PIN, INPUT);
  attachInterrupt(INPUT_PIN, event, RISING);


  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);

  // setSyncProvider(getNtpTime);
  // setSyncInterval(1);

  mailman.attach(0.5, sendEvent);
}

void loop()
{
  // Serial.print(digitalRead(INPUT_1));
  // Serial.println(digitalRead(INPUT_2));

  // if (!client.connected())
  // {
  //   time_t now = millis();
  //   if (now - lastReconnectAttempt > 5000)
  //   {
  //     lastReconnectAttempt = now;
  //     // Attempt to reconnect
  //     if (reconnect())
  //     {
  //       lastReconnectAttempt = 0;
  //     }
  //   }
  // }
  // else
  // {
  //   // Client connected
  //   client.loop();
  // }

  // // Update time
  // if (timeStatus() != timeNotSet)
  // {
  //   if (now() != currentTime)
  //   { //update the display only if time has changed
  //     currentTime = now();
  //     digitalClockDisplay();
  //   }
  // }
}