#include <SoftwareSerial.h>
#include <ESP8266.h>

#define SSID "sloniki"
#define PASS "MyOwnBlend"

#ifdef MEGA
#define wifi_serial Serial1
#else
#define wifi_serial Serial
#endif

ESP8266 wifi(wifi_serial);

void setup()
{
	dprintln("init");
	delay(1000);


	dprint("begin(): ");
	wifi.set_reset_pin(2);
	uint8_t result = wifi.begin();
	dprintln(result);
	if (!result)
		return;

	dprint("connect(): ");
	dprintln(wifi.connect(SSID, PASS));

	char *ip = (char *)wifi.getIP();
	dprint("getIP(): ");
	dprintln(ip);
}

void loop() {
  dprintln("mloop");
  delay(1000);
  while (wifi_serial.available()) {
	dprintln("readString");
    String s = wifi_serial.readString();
    dprintln("WIFI: '" + s + "'");
  }
  while(DEBUG_SERIAL.available()) {
    String s = DEBUG_SERIAL.readString();
//	DEBUG_SERIAL.println(s);
    wifi_serial.println(s);
  }
  
}
