/*
* @Author: Ramon Melo
* @Date:   2018-08-18
* @Last Modified by:   Ramon Melo
* @Last Modified time: 2018-08-18
*/

#define SLOG Serial.print
#define SLOGN Serial.println

#define READ_RATE 1000
#define RECONNECT_RATE 5000

#include "mes_defines.h"

#include "TypeSelector.h"
#include "LedManager.h"

#include <ESP8266WiFi.h>

#include "Arduino.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>

enum BlinkerState
{
	BROKER,
	WIFI,
	CONFIG,
	TYPE1,
	TYPE2
};

unsigned long nextReconnectAttempt = 0;
unsigned long nextSend = 0;

int selectedType = 0;
volatile byte packageCount = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// volatile std::queue<int> eventQueue;

mes::TypeSelector typeSelector(INPUT_1, INPUT_2);
mes::LedManager ledManager(LED_RED, LED_GREEN, LED_BLUE);

bool status = false;
unsigned long last_up = 0;
bool long_blink = false;

void blink_wifi(int delay)
{
	if (WiFi.status() != WL_CONNECTED)
	{
		if (millis() > last_up)
		{
			status = !status;
			last_up = millis() + delay;
		}

		if (status)
		{
			ledManager.red();
		}
		else
		{
			ledManager.black();
		}
	}
	else
	{
		ledManager.black();
	}
}

void blinker(BlinkerState state)
{
	if (long_blink)
	{
		if (millis() > last_up)
		{
			long_blink = false;
		}

		return;
	}

	switch (state)
	{
	case WIFI:
		blink_wifi(100);
		break;
	case CONFIG:
		blink_wifi(50);
		break;
	case BROKER:
		if (client.connected())
		{
			ledManager.white();
		}
		else
		{
			ledManager.red();
		}
		break;
	case TYPE1:
		long_blink = true;
		last_up = millis() + 500;
		ledManager.blue();

		break;
	case TYPE2:
		long_blink = true;
		last_up = millis() + 500;
		ledManager.green();

		break;
	default:
		ledManager.black();
	}
}

void connect()
{
#if defined(WIFI_SSID) && defined(WIFI_PASS)
	WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif

	WiFi.mode(WIFI_STA);

	bool success = false;

	SLOG("\nConnecting...");

	long start_time = millis();
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(50);

		if ((millis() - start_time > 10000) && !success)
		{
			WiFi.beginSmartConfig();
			SLOG("\nBegin SmartConfig...");

			while (1)
			{
				delay(50);

				if (WiFi.smartConfigDone())
				{
					SLOG("\nSmartConfig: Success");

					success = true;
					break;
				}

				blinker(BlinkerState::CONFIG);
			}
		}

		blinker(BlinkerState::WIFI);
	}

	SLOG("WiFi Connected.");
	WiFi.printDiag(Serial);
	SLOG("IP Address: ");
	SLOGN(WiFi.localIP().toString().c_str());

	SLOG("Gateway IP: ");
	SLOGN(WiFi.gatewayIP().toString().c_str());

	SLOG("Hostname: ");
	SLOGN(WiFi.hostname().c_str());

	SLOG("RSSI: ");
	SLOG(WiFi.RSSI());
	SLOGN(" dbm");
}

void buildMessage(String *jsonStr, char type)
{
	const size_t bufferSize = JSON_OBJECT_SIZE(3);
	DynamicJsonBuffer jsonBuffer(bufferSize);

	JsonObject &root = jsonBuffer.createObject();
	root["p"] = type;
	root["i"] = MES_DEVICE_ID;

	root.printTo(*jsonStr);
}

void sendEvent()
{
	if (packageCount > 0 && client.connected() == true)
	{
		String msg;
		buildMessage(&msg, selectedType);

		client.publish(MQTT_TOPIC, msg.c_str());
		Serial.print("_");

		packageCount--;
	}
}

void event()
{
	if (nextSend < millis())
	{
		// int selectedType = typeSelector.currentType();

		// if (selectedType == 0)
		// {
		// 	client.publish("/error", "No type selected");
		// 	return;
		// }

		// switch (selectedType)
		// {
		// case 1:
		// 	blinker(BlinkerState::TYPE1);
		// 	break;
		// case 2:
		// 	blinker(BlinkerState::TYPE2);
		// 	break;
		// }

		// eventQueue.push(selectedType);

		packageCount++;

		nextSend = millis() + READ_RATE;
	}
}

boolean reconnect()
{
	SLOGN("Reconnecting");

#ifdef MQTT_AUTH
	if (client.connect(MES_DEVICE_ID, MQTT_USER, MQTT_PASS, "/info", 2, false, "Hello"))
#else
	if (client.connect(MES_DEVICE_ID, "/info", 2, false, "Hello"))
#endif
	{
		// Once connected, publish an announcement...
		client.publish("/info", "I'm Alive!");
	}

	return client.connected();
}

void setup()
{
	ledManager.red();

	Serial.begin(115200);
	while (!Serial)
	{
	}

	SLOGN("Booted");

	connect();
	client.setServer(MQTT_BROKER, MQTT_PORT);

	pinMode(INPUT_PIN, INPUT_PULLUP);
	attachInterrupt(INPUT_PIN, event, RISING);
}

void loop()
{
	blinker(BlinkerState::BROKER);

	if (client.connected() == false)
	{
		if (nextReconnectAttempt < millis())
		{
			nextReconnectAttempt = millis() + RECONNECT_RATE;
			reconnect();
		}
	}

	client.loop();
	sendEvent();
}