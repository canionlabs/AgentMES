/*
* @Author: Ramon Melo
* @Date:   2018-08-18
* @Last Modified by:   Ramon Melo
* @Last Modified time: 2018-08-18
*/

#define UPDATE_RATE 60 // seconds

#define LED 2
#define LED_GREEN 12
#define LED_BLUE 13
#define LED_RED 15

#define PIN_INPUT D1

#include "Arduino.h"
#include <NTPtimeESP.h>
#include <PubSubClient.h>
#include <TimeLib.h>
#include <vector>

#include "config.h"

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

std::vector<Event> eventList;

time_t getNtpTime()
{
  Serial.println("Try to get date");

  do {
    currentDate = NTPch.getNTPtime(-3.0, 0);
    delay(50);
  } while(!currentDate.valid);

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
  Serial.println("Connecting to Wi-Fi");

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

void event()
{
  Serial.print("?");

  if ((currentTime - lastSend) > sendRate)
  {
    lastSend = currentTime;
    Serial.print("|");

    if (client.connected())
    {
      client.publish(MQTT_TOPIC, "ping");
      Serial.print(".");
    }
  }
}

boolean reconnect()
{
  if (client.connect(MES_DEVICE_ID, MQTT_USER, MQTT_PASS))
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

  Serial.println();
  Serial.println("Booted");

  connect();
  client.setServer(MQTT_BROKER, MQTT_PORT);

  pinMode(PIN_INPUT, INPUT);
  attachInterrupt(PIN_INPUT, event, RISING);

  setSyncProvider(getNtpTime);
  setSyncInterval(1);
}

void loop()
{
  if (!client.connected())
  {
    time_t now = millis();
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    // Client connected
    client.loop();
  }

  // Update time
  if (timeStatus() != timeNotSet)
  {
    if (now() != currentTime)
    { //update the display only if time has changed
      currentTime = now();
      digitalClockDisplay();
    }
  }
}