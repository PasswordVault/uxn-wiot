#include "config.h"
#ifdef USE_WIO_TERMINAL
#include <Arduino.h>

int specific_boot() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("UXN on Wio Terminal");
    return 1;
}

#endif