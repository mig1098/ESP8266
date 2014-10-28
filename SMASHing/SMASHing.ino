#include <ESP8266.h>

ESP8266 wifi(Serial1);

void setup() {
  Serial.begin(115200); // Serial monitor
  Serial1.begin(115200); // ESP8266

  Serial.println("ESP8266 SMASHing demo.");

  Serial.print("begin(): ");
  Serial.println(wifi.begin());

  Serial.print("connect(): ");
  Serial.println(wifi.connect("Belkin.8927", "YOLOSWAG"));

  Serial.print("getIP(): ");
  Serial.println(wifi.getIP());
}

void loop() {

}
