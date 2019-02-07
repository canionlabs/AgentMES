
#include <Arduino.h>

namespace mes {

  class LedManager {
  public:
    LedManager(int LED_R, int LED_G, int LED_B);

    void white();
    void black();
    void blue();
    void red();
    void cyan();
    void green();

  private:
    int led_r;
    int led_g;
    int led_b;
  };

}