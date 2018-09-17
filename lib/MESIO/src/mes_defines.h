// ███╗   ███╗███████╗███████╗
// ████╗ ████║██╔════╝██╔════╝
// ██╔████╔██║█████╗  ███████╗
// ██║╚██╔╝██║██╔══╝  ╚════██║
// ██║ ╚═╝ ██║███████╗███████║
// ╚═╝     ╚═╝╚══════╝╚══════╝
// Definitions
//

// Device

#define INPUT_PIN 5

#define INPUT_1 4
#define INPUT_2 14

#define LED 2
#define LED_GREEN 12
#define LED_BLUE 13
#define LED_RED 15

// Cloud

// #define MQTT_AUTH
// #define MQTT_BROKER "broker.shiftr.io"
#define MQTT_BROKER "autopolar.duckdns.org"
#define MQTT_PORT 1883
#define MQTT_TOPIC "canionlabs/autopolar"
// #define MQTT_USER "71b18f70"
// #define MQTT_PASS "5daf5773ea30f228"

// MES

#define MES_SERVER "http://mesadmin.canionlabs.io/api/receiver/"
#define MES_ORGANIZATION "canionlabs001"
#define MES_DEVICE_TYPE "esp8266"
#define MES_DEVICE_ID "autopolar-gelo"

// Connection
// #define WIFI_SSID "Automata"
// #define WIFI_PASS "data.hal.johnny"