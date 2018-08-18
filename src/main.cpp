/*
* @Author: Ramon Melo
* @Date:   2018-08-18
* @Last Modified by:   Ramon Melo
* @Last Modified time: 2018-08-18
*/

#define APP_DEBUG
#define BLYNK_PRINT Serial // Comment out when deploy
#define BLYNK_NO_BUILTIN
#define BLYNK_NO_FLOAT
#define USE_CUSTOM_BOARD

#define UPDATE_RATE 60 // seconds

#include "Arduino.h"
#include <BlynkSimpleEsp8266.h>
#include <BlynkProvisioning.h>
#include <TimeLib.h>

BlynkTimer timer;

void service()
{
}

void setup() {
  delay(500);
  Serial.begin(9600);

  BlynkProvisioning.begin();
  timer.setInterval(1000L * UPDATE_RATE, service);

  BLYNK_LOG("done!");
}

void loop() {
  BlynkProvisioning.run();
  timer.run();
}
