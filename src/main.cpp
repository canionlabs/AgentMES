/*
* @Author: Ramon Melo
* @Date:   2018-08-18
* @Last Modified by:   Ramon Melo
* @Last Modified time: 2018-08-18
*/

#define READ_RATE 2000
#define RECONNECT_RATE 5000
#define CONNECT_WAIT 30 * 1000 // 30 sec

#include "mes_defines.h"

// #include "TypeSelector.h"
// #include "LedManager.h"

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

enum CurrState
{
	INIT,
	WIFI_DISCONNECTED,
	WIFI_CONNECTED,
	BROKER_DISCONNECTED,
	BROKER_CONNECTED,
	SMART_CONFIG,
};

CurrState state = CurrState::INIT;

unsigned long nextReconnectAttempt = 0;
unsigned long nextSend = 0;

char selectedType = 'A';
volatile byte packageCount = 0;

WiFiClient espClient;
PubSubClient client(espClient, MQTT_BROKER, MQTT_PORT);

Adafruit_SSD1306 display;

// mes::TypeSelector typeSelector(INPUT_1, INPUT_2);
// mes::LedManager ledManager(LED_RED, LED_GREEN, LED_BLUE);

bool status = false;
unsigned long last_up = 0;
bool long_blink = false;

void write(int x, int y, int size, String text)
{
	display.setCursor(x, y);
	display.setTextSize(size);
	display.setTextColor(WHITE);
	display.print(text);
}

void updateView() // int current, int total, float value, float avg, String addr, int percent
{
	display.clearDisplay();

	switch (state)
	{
	case CurrState::INIT:
		write(2, 0, 1, ": INICIANDO...");
		break;
	case CurrState::WIFI_DISCONNECTED:
		write(2, 0, 1, ": WIFI DESCONECTADO");
		break;
	case CurrState::WIFI_CONNECTED:
		write(2, 0, 1, ": WIFI CONECTADO");
		break;
	case CurrState::SMART_CONFIG:
		write(2, 0, 1, ": SMART CONFIG");
		break;
	case CurrState::BROKER_DISCONNECTED:
		write(2, 0, 1, ": SERVIDOR DESCONECTADO");
		break;
	case CurrState::BROKER_CONNECTED:
		write(2, 0, 1, ": SERVIDOR CONECTADO");
		break;
	default:
		write(2, 0, 1, "...");
		break;
	}

	write(0, 10, 3, "T:" + String(selectedType));

	// write(0, 0, 1, "hello world");
	// write(0, 10, 1, addr);
	// write(60, 0, 1, "AVG: " + String(avg));
	// write(38, 9, 3, String(value));

	// display.drawLine(percent, display.height() - 1, display.width() - 1, display.height() - 1, WHITE);

	display.display();
}

void wifiConnect()
{
	if (WiFi.status() == WL_CONNECTED)
	{
		return;
	}

#if defined(WIFI_SSID) && defined(WIFI_PASS)
	WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif

	WiFi.mode(WIFI_STA);

	state = CurrState::WIFI_DISCONNECTED;
	updateView();

	unsigned long wait_time = millis() + CONNECT_WAIT;

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(50);

		// wait 30 sec to connect to wifi
		if (wait_time < millis())
		{
			// enter SmartConfig mode
			WiFi.beginSmartConfig();
			state = CurrState::SMART_CONFIG;
			updateView();

			wait_time = millis() + CONNECT_WAIT;

			while (true)
			{
				delay(50);

				if (WiFi.smartConfigDone())
				{
					break;
				}

				// wait 30 sec on smart config
				if (wait_time < millis())
				{
					// restart if smart config fails
					ESP.restart();
				}
			}
		}
	}

	state = CurrState::WIFI_CONNECTED;
	updateView();
}

void buildMessage(String *jsonStr, char type)
{
	const size_t bufferSize = JSON_OBJECT_SIZE(2);
	DynamicJsonBuffer jsonBuffer(bufferSize);

	JsonObject &root = jsonBuffer.createObject();
	root["p"] = type;
	root["i"] = MES_DEVICE_ID;

	root.printTo(*jsonStr);
}

void cfgHandler(const MQTT::Publish& pub)
{
	if (strcmp(pub.topic().c_str(), MES_CFG_TOPIC) == 0)
	{
		selectedType = (char) pub.payload()[0];

		client.publish(MQTT::Publish(MES_CFG_CONFIRM_TOPIC, pub.payload(), pub.payload_len()).set_qos(2));
	}
}

// void cfgHandler(char *topic, byte *payload, unsigned int length)
// {
// 	if (strcmp(topic, MES_CFG_TOPIC) == 0)
// 	{
// 		selectedType = (char)payload[0];

// 		// Allocate the correct amount of memory for the payload copy
// 		byte *p = (byte *)malloc(length);
// 		// Copy the payload to the new buffer
// 		memcpy(p, payload, length);
// 		client.publish(MES_DATA_TOPIC, p, length);

// 		// Free the memory
// 		free(p);
// 	}
// }

void sendEvent()
{
	if (packageCount > 0 && client.connected() == true)
	{
		String msg;
		buildMessage(&msg, selectedType);

		client.publish(MQTT::Publish(MES_DATA_TOPIC, msg.c_str()).set_qos(2));

		packageCount--;
	}
}

void event()
{
	if (nextSend < millis())
	{
		packageCount++;
		nextSend = millis() + READ_RATE;
	}
}

void brokerConnect()
{
	if (client.connected() || nextReconnectAttempt > millis())
	{
		return;
	}

	state = CurrState::BROKER_DISCONNECTED;
	updateView();

#if defined(MQTT_USER) && defined(MQTT_PASS)
	if (client.connect(MQTT::Connect(MES_DEVICE_ID).set_will(MES_WILL_TOPIC, MES_DEVICE_ID, 2, false).set_auth(MQTT_USER, MQTT_PASS)))
#else
	if (client.connect(MQTT::Connect(MES_DEVICE_ID).set_will(MES_WILL_TOPIC, MES_DEVICE_ID, 2, false)))
#endif
	{
		state = CurrState::BROKER_CONNECTED;
		updateView();

		// Once connected, publish an announcement...
		client.publish(MES_STATE_TOPIC, MES_DEVICE_ID);

		client.subscribe(MES_CFG_TOPIC);
	}

	nextReconnectAttempt = millis() + RECONNECT_RATE;
}

void setup()
{
	Serial.begin(115200);
	while (!Serial)
	{
	}

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

	state = CurrState::INIT;
	updateView();

	client.set_callback(cfgHandler);

	pinMode(INPUT_PIN, INPUT_PULLUP);
	attachInterrupt(INPUT_PIN, event, CHANGE);

	Serial.println("ready");
}

void loop()
{
	wifiConnect();
	brokerConnect();

	client.loop();
	sendEvent();

	updateView();

	// Little Delay
	delay(10);
}