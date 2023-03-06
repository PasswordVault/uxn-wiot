#include <Arduino.h>
#include "config.h"

#ifdef USE_SPIFFS
#include <SPIFFS.h>
static char *rom = "/spiffs/ufo.rom";
#endif

#ifdef USE_SDCARD
//#include <Seeed_Arduino_FS.h>
#include <SPI.h>
#include <SD.h>
static char *rom = "draw.rom";
#endif

extern "C" {
  #include <uxn.h>
  #include <devices/file.h>
  #include <devices/screen.h>
  #include <devices/system.h>
}

#ifdef USE_MONITOR
#include "monitor.h"
#endif

/********** Config ***********/
const char* ntp_server = "pool.ntp.org";
const long gmt_offset_sec = 3600;
const int daylight_offset_sec = 3600;

/*****************************/

#ifdef USE_WIFI
#include "WiFi.h"
#include "wifi_credentials.h"
#endif
#include "time.h"

int specific_boot();
int devscreen_init();
int devscreen_redraw();
int devctrl_init();
int devctrl_handle(Device *d);
int devmouse_init();
int devmouse_handle(Device *d);

static Uxn u;
static Device *devsystem, *devconsole, *devscreen, *devctrl, *devmouse;

void
error(const char *msg, const char *err)
{
  Serial.printf("Error %s: %s\n", msg, err);
  /*
  while(1)
    delay(1000);
  */
}

void
system_deo_special(Device *d, Uint8 port)
{
	if(port > 0x7 && port < 0xe)
		screen_palette(&uxn_screen, &d->dat[0x8]);
}

static void
console_deo(Device *d, Uint8 port)
{
	if(port == 0x1)
		DEVPEEK16(d->vector, 0x0);
	if(port > 0x7) {
#ifdef ARDUINO
		Serial.print((char *)&d->dat[port]);
#else
		write(port - 0x7, (char *)&d->dat[port], 1);
#endif
	}
}

static Uint8
datetime_dei(Device *d, Uint8 port)
{
	time_t seconds = time(NULL);
	struct tm zt = {0};
	struct tm *t = localtime(&seconds);
	if(t == NULL)
		t = &zt;
	switch(port) {
	case 0x0: return (t->tm_year + 1900) >> 8;
	case 0x1: return (t->tm_year + 1900);
	case 0x2: return t->tm_mon;
	case 0x3: return t->tm_mday;
	case 0x4: return t->tm_hour;
	case 0x5: return t->tm_min;
	case 0x6: return t->tm_sec;
	case 0x7: return t->tm_wday;
	case 0x8: return t->tm_yday >> 8;
	case 0x9: return t->tm_yday;
	case 0xa: return t->tm_isdst;
	default: return d->dat[port];
	}
}

static Uint8
nil_dei(Device *d, Uint8 port)
{
	return d->dat[port];
}

static void
nil_deo(Device *d, Uint8 port)
{
	if(port == 0x1) DEVPEEK16(d->vector, 0x0);
}


void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      //printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

int
load(Uxn *u, char *filepath)
{
#ifdef USE_SPIFFS
	FILE *f;
	int r;
	if(!(f = fopen(filepath, "rb"))) return 0;
	r = fread(u->ram + PAGE_PROGRAM, 1, 0xffff - PAGE_PROGRAM, f);
	fclose(f);
	if(r < 1) return 0;
#endif

#ifdef USE_SDCARD
	Serial.printf("Loading ROM %s\n", filepath);
	if (!SD.begin(SDCARD_SS_PIN)) {
		Serial.println("Initialization failed!");
		return 0;
	}

	File root = SD.open("/");
	printDirectory(root, 0);

	if (!SD.exists(filepath)) {
		Serial.println("ROM does not exist");
		return 0;
	}
	File f = SD.open(filepath);
	int r = f.read(u->ram + PAGE_PROGRAM, 0xffff - PAGE_PROGRAM);
  	f.close();
	if(r < 1) return 0;
#endif

	Serial.printf("Loaded %d bytes from %s\n", r, filepath);
	return 1;
}

void
run(Uxn *u)
{
  bool running = false;

  char c;
  unsigned long ts, elapsed;
  Serial.println("Running...");
  devscreen_redraw();
  while (true) {
	Serial.println("Loop...");
	while(!devsystem->dat[0xf]) {
		ts = micros();
		if(Serial.available() > 0) {
			Serial.readBytes(&c, 1);
			devconsole->dat[0x2] = c;
			if(!uxn_eval(u, devconsole->vector))
				error("Console", "eval failed");
		}
		devctrl_handle(devctrl);
	#ifdef USE_TOUCH_SCREEN
		devmouse_handle(devmouse);
	#endif
		uxn_eval(u, devscreen->vector);
		if(uxn_screen.changed || devsystem->dat[0xe])
			devscreen_redraw();
		elapsed = micros() - ts;
		if(elapsed < 16666)
			delayMicroseconds(16666 - elapsed);

	}
	while (devsystem->dat[0xf]) {
		if (Serial.available()) {
			mon_onechar(u, Serial.read());
		}
	}

  }
}

void setup() {
  specific_boot();

  devscreen_init();
  devctrl_init();

/* TODO: put this in a background task so we don't have to wait for the connection */
#ifdef USE_WIFI
  Serial.printf("Connecting to \"%s\"", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
	  delay(500);
	  Serial.print(".");
  }
  WiFi.setAutoReconnect(true);
  Serial.println("Connected.");
  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);
#endif
  
#ifdef USE_SPIFFS
  SPIFFS.begin();
#endif

  Uint8 *memory = (Uint8 *)calloc(0xffff, sizeof(Uint8));
  if(memory == NULL)
	error("Boot", "Not enough memory");
  if(!uxn_boot(&u, memory))
	error("Boot", "Failed to start uxn.");

    /* system     */ devsystem = uxn_port(&u, 0x0, system_dei, system_deo);
	/* console    */ devconsole = uxn_port(&u, 0x1, nil_dei, console_deo);
	/* screen     */ devscreen = uxn_port(&u, 0x2, screen_dei, screen_deo);
	/* empty      */ uxn_port(&u, 0x3, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0x4, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0x5, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0x6, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0x7, nil_dei, nil_deo);
	/* control    */ devctrl = uxn_port(&u, 0x8, nil_dei, nil_deo);
	/* mouse      */ devmouse = uxn_port(&u, 0x9, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0xa, nil_dei, file_deo);
	/* datetime   */ uxn_port(&u, 0xb, datetime_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0xc, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0xd, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0xe, nil_dei, nil_deo);
	/* empty      */ uxn_port(&u, 0xf, nil_dei, nil_deo);

  Serial.println("Loading ROM...");
  if(!load(&u, rom))
    error("Load", "Failed");

  Serial.println("Running...");
  if(!uxn_eval(&u, PAGE_PROGRAM))
    error("Init", "Failed");

  run(&u);
}

void loop() {
}