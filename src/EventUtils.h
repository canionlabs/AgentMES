#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

void buildMessage(String *jsonStr, char type, time_t eventTime)
{
  const size_t bufferSize = JSON_OBJECT_SIZE(3);
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject &root = jsonBuffer.createObject();
  root["p"] = type;
  root["t"] = eventTime;
  root["i"] = MES_DEVICE_ID;

  root.printTo(*jsonStr);
}