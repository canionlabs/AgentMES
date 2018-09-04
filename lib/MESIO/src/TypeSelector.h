
#include <Arduino.h>

namespace mes {

  class TypeSelector {

  public:
    TypeSelector(int PIN1, int PIN2);
    int currentType();
  private:
    int pin1;
    int pin2;
  };

}