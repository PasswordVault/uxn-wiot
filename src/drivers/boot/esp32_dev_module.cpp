#include "config.h"
#ifdef USE_ESP32_DEV_MODULE_BOOT
#include <Arduino.h>

int specific_boot() {
    Serial.begin(115200);
    devmouse_init();
    return 1;
}

#endif