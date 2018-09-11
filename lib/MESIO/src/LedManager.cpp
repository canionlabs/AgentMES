
#include "LedManager.h"

namespace mes
{
  LedManager::LedManager(int LED_R, int LED_G, int LED_B) : led_r(LED_R), led_g(LED_G), led_b(LED_B) {
    pinMode(led_r, OUTPUT);
    pinMode(led_g, OUTPUT);
    pinMode(led_b, OUTPUT);

    digitalWrite(led_r, LOW);
    digitalWrite(led_g, LOW);
    digitalWrite(led_b, LOW);
  }

  void LedManager::white() {
    digitalWrite(led_r, HIGH);
    digitalWrite(led_g, HIGH);
    digitalWrite(led_b, HIGH);
  }

  void LedManager::black() {
    digitalWrite(led_r, LOW);
    digitalWrite(led_g, LOW);
    digitalWrite(led_b, LOW);
  }

  void LedManager::blue() {
    digitalWrite(led_r, LOW);
    digitalWrite(led_g, LOW);
    digitalWrite(led_b, HIGH);
  }

  void LedManager::red() {
    digitalWrite(led_r, HIGH);
    digitalWrite(led_g, LOW);
    digitalWrite(led_b, LOW);
  }

  void LedManager::green() {
    digitalWrite(led_r, LOW);
    digitalWrite(led_g, HIGH);
    digitalWrite(led_b, LOW);
  }

} // mes
