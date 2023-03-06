#include "config.h"
#ifdef USE_M5CORE2_BOOT
#include <M5Core2.h>

int specific_boot() {
    M5.begin();
    devmouse_init();
    return 1;
}

#endif