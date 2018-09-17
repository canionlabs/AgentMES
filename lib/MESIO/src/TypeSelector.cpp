
#include "TypeSelector.h"

namespace mes {

  TypeSelector::TypeSelector(int PIN1, int PIN2) : pin1(PIN1), pin2(PIN2) {
    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);
  }

  int TypeSelector::currentType() {
    if ( digitalRead(pin1) == 0 ) {
      return 1;
    }

    if ( digitalRead(pin2) == 0 ) {
      return 2;
    }

    return 0;
  }
}