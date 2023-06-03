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

// Cloud

#define MQTT_BROKER "canionlabsmes.local"
#define MQTT_PORT 1883
#define MQTT_USER "canionlabs_user"
#define MQTT_PASS "canionlabs_pass"
#define MQTT_QOS 2

// Communication

#define MES_STATE_TOPIC "/canionlabs/mes/status"
#define MES_CFG_TOPIC "/canionlabs/mes/cfg"
#define MES_CFG_CONFIRM_TOPIC "/canionlabs/mes/cfg/confirm"
#define MES_DATA_TOPIC "/canionlabs/mes/data"
#define MES_WILL_TOPIC "/canionlabs/mes/will"
#define MES_PING_TOPIC "/canionlabs/mes/ping"

// MES

#define MES_DEVICE_ID "kalfix"

// Connection
#define PORTAL_PW "@#canionlabs@#"
#define PORTAL_TITLE "MONAR"