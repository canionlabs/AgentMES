// ███╗   ███╗███████╗███████╗
// ████╗ ████║██╔════╝██╔════╝
// ██╔████╔██║█████╗  ███████╗
// ██║╚██╔╝██║██╔══╝  ╚════██║
// ██║ ╚═╝ ██║███████╗███████║
// ╚═╝     ╚═╝╚══════╝╚══════╝
// Definitions
//

// Device

#define INPUT_PIN 15

// #define INPUT_1 4
// #define INPUT_2 14

// #define LED 2
// #define LED_GREEN 12
// #define LED_BLUE 13
// #define LED_RED 15

// Cloud

#define MQTT_BROKER "canionlabsmes.local"
#define MQTT_PORT 1883
#define MQTT_USER "71b18f70"
#define MQTT_PASS "5daf5773ea30f228"
#define MQTT_QOS 1

// Communication

#define MES_STATE_TOPIC "canionlabs/mes/status"
#define MES_CFG_TOPIC "canionlabs/mes/cfg"
#define MES_CFG_CONFIRM_TOPIC "canionlabs/mes/cfg/confirm"
#define MES_DATA_TOPIC "canionlabs/mes/data"
#define MES_WILL_TOPIC "canionlabs/mes/will"

// MES

// #define MES_SERVER "http://mesadmin.canionlabs.io/api/receiver/"
// #define MES_ORGANIZATION "canionlabs001"
// #define MES_DEVICE_TYPE "esp8266"
#define MES_DEVICE_ID "kalfix"

// Connection
#define PORTAL_PW "@#canionlabs@#"
#define PORTAL_TITLE "MONAR"