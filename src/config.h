#ifndef CONFIG_H
#define CONFIG_H

//#define HAS_UXN_FILE

//#define USE_WIFI

//#define USE_ESP32_DEV_MODULE_BOOT
//#define USE_M5CORE2_BOOT
#define USE_WIO_TERMINAL

/********* devscreen *********/
#define USE_TFT_ESPI
//#define USE_TFT_DMA
//#define USE_M5CORE2_SCREEN
/*****************************/

/********** devctrl **********/
//#define USE_NUNCHUCK
//#define USE_SERIAL2
//#define USE_M5CORE2_BUTTONS
#define USE_WIOT_BUTTONS
/*****************************/

/********** devmouse *********/
//#define USE_TOUCH_SCREEN
//#define CAL_DATA { 336, 3549, 262, 3517, 1 }; /* Use Touch_calibrate example of TFT_eSPI lib to calibrate the touch screen */
//#define USE_M5CORE2_TOUCH
/*****************************/

/********** main *************/
//#define USE_SPIFFS
#define USE_SDCARD
#define USE_MONITOR
/*****************************/

#endif