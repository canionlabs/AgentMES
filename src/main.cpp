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
#include "Arduino.h"

// AutoConnect
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <AutoConnect.h>

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

char selectedType = 'A';
volatile unsigned long nextSend = 0;
volatile byte packageCount = 0;

WiFiClient espClient;
PubSubClient client(espClient, MQTT_BROKER, MQTT_PORT);

ESP8266WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;
Adafruit_SSD1306 display;

bool status = false;
unsigned long last_up = 0;
bool long_blink = false;

void rootPage()
{
	char content[] = "MES, CanionLabs";
	Server.send(200, "text/plain", content);
}

void write(int x, int y, int size, String text)
{
	display.setCursor(x, y);
	display.setTextSize(size);
	display.setTextColor(WHITE);
	display.print(text);
}

void updateView()
{
	display.clearDisplay();

	switch (state)
	{
	case CurrState::INIT:
		write(2, 0, 1, ": INICIANDO...");
		break;
	case CurrState::WIFI_DISCONNECTED:
		write(2, 0, 1, ": WIFI OFF");
		break;
	case CurrState::WIFI_CONNECTED:
		write(2, 0, 1, ": WIFI ON");
		break;
	case CurrState::SMART_CONFIG:
		write(2, 0, 1, ": SMART CONFIG");
		break;
	case CurrState::BROKER_DISCONNECTED:
		write(2, 0, 1, ": SERVIDOR OFF");
		break;
	case CurrState::BROKER_CONNECTED:
		write(2, 0, 1, ": SERVIDOR ON");
		break;
	default:
		write(2, 0, 1, "...");
		break;
	}

	write(0, 10, 3, "T:" + String(selectedType));
	display.display();
}

void buildMessage(String *jsonStr, char type)
{
	const size_t bufferSize = JSON_OBJECT_SIZE(2);
	DynamicJsonDocument jsonBuffer(bufferSize);
	jsonBuffer["p"] = type;
	jsonBuffer["i"] = MES_DEVICE_ID;

	serializeJson(jsonBuffer, *jsonStr);
}

void cfgHandler(const MQTT::Publish &pub)
{
	if (strcmp(pub.topic().c_str(), MES_CFG_TOPIC) == 0)
	{
		selectedType = (char)pub.payload()[0];

		client.publish(MQTT::Publish(MES_CFG_CONFIRM_TOPIC, pub.payload(), pub.payload_len()).set_qos(2));
	}
}

/**
 * @brief  Publishes to broker that a package has been checked
 * @note   This function will only perform the publish if a package was registered by the sensor
 * @retval None
 */
void sendEvent()
{
	if (packageCount > 0 && client.connected() == true)
	{
		String msg;
		buildMessage(&msg, selectedType);

		client.publish(MQTT::Publish(MES_DATA_TOPIC, msg.c_str()).set_qos(MQTT_QOS));

		packageCount--;
		Serial.println("send pkg");
	}
}

/**
 * @brief  Event Interrupt that will be invoked every time the sensor detects something passing by
 * @retval None
 */
ICACHE_RAM_ATTR void event(void)
{
	if (nextSend < millis())
	{
		packageCount++;
		nextSend = millis() + READ_RATE;
	}
}

/**
 * @brief  This method tries to connect this client to MQTT Broker
 * @note   If the client is already connected, just return, if not, it will try to connect to the broker every 
 *         RECONNECT_RATE millis
 * @retval None
 */
void brokerConnect()
{
	if (client.connected() || nextReconnectAttempt > millis())
	{
		return;
	}

	state = CurrState::BROKER_DISCONNECTED;
	updateView();

#if defined(MQTT_USER) && defined(MQTT_PASS)
	if (client.connect(
		MQTT::Connect(MES_DEVICE_ID)
		.set_will(MES_WILL_TOPIC, MES_DEVICE_ID, 2, false)
		.set_auth(MQTT_USER, MQTT_PASS)
	))
#else
	if (client.connect(MQTT::Connect(MES_DEVICE_ID).set_will(MES_WILL_TOPIC, MES_DEVICE_ID, 2, false)))
#endif
	{
		state = CurrState::BROKER_CONNECTED;
		updateView();

		// Once connected, publish an announcement...
		client.publish(MQTT::Publish(MES_STATE_TOPIC, MES_DEVICE_ID).set_qos(MQTT_QOS));

		// Subscribe to Config topic
		client.subscribe(MQTT::Subscribe().add_topic(MES_CFG_TOPIC, MQTT_QOS));
	}

	nextReconnectAttempt = millis() + RECONNECT_RATE;
}

void setup()
{
	Serial.begin(115200);
	while (!Serial) {}

	state = CurrState::INIT;

	// Start Display
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	updateView();
	// END Start Display

	// Configure MQTT Client
	client.set_callback(cfgHandler);
	// END Configure MQTT Client

	// Configure Sensor Input
	pinMode(INPUT_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(INPUT_PIN), event, RISING);
	// END Configure Sensor Input

	// Configure AutoConnect
	Config.title = PORTAL_TITLE;
	Config.apid = PORTAL_TITLE + WiFi.macAddress();
	Config.psk = PORTAL_PW;
	Config.autoReconnect = true;

	Portal.config(Config);

	Server.on("/", rootPage);
	if (Portal.begin())
	{
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
	}
	else
	{
		Serial.println("Error starting portal");
	}
	// END Configure AutoConnect

	Serial.println("ready");
}

void loop()
{
	brokerConnect();

	client.loop();
	sendEvent();

	updateView();

	// Little Delay
	delay(10);
}