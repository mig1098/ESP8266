//#include <SoftwareSerial.h>
#include <ESP8266.h>
#include <ESP8266Funcs.h>
#include <BufferString.h>

#define SSID "SSID"
#define PASS "PASS"

#ifdef MEGA
#define wifi_serial Serial1
#else
#define wifi_serial Serial
#endif

ESP8266 wifi(wifi_serial);

void setup()
{
	delay(1000);

	wifi.set_reset_pin(4);
	
	for (uint8_t digitalPin = 2; digitalPin <= 9; ++digitalPin) {
		pinMode(digitalPin, INPUT); //bug workaround (http://forum.arduino.cc/index.php?topic=185291.0) 
		pinMode(digitalPin, OUTPUT);
		digitalWrite(digitalPin, HIGH);
	}

	while(1) {
		dprint("begin(): ");
		uint8_t result = wifi.begin();
		dprintln(result);
		if (!result)
			continue;

		dprint("connect(): ");
		if(!wifi.connect(SSID, PASS))
			continue;

		char *ip = (char *)wifi.getIP();
		dprint("getIP(): ");
		dprintln(ip);

		dprintln("serv");
		if(!wifi.confServer())
			continue;

		break;
	}
}

void loop() {
	dprintln("loop");
	char *msg = (char*)wifi.ReceiveMessage();

	if (msg) {
		//dprint("MSG: '");
		//dprint(msg);
		//dprintln("'");

		http_reply(msg, strlen(msg));
	}

/*	dprintln("mloop");
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
*/
}

void http_reply(char *buf, int len) {
	for(int i = 0; i < len - 2; ++i) {
		if (!(strncmp(buf, "GET", 3)))
			break;
		++buf;
	}
	buf += 4;
//	while(*buf == ' ') ++buf;

	char *q_end = buf;
	while(*q_end != ' ' && *q_end != '\r' && *q_end != '\n') ++q_end;
	*q_end = '\0';

	dispatch(buf);
}


const char* SET_QUERY = "/set?";

/* receives url as a param */
void dispatch(char *query) {
	if(!strcmp(query, "/")) {
		reply_sensor_data();
	} else if(!strncmp(query, SET_QUERY, sizeof(SET_QUERY))) {
		set_cmd(query + strlen(SET_QUERY));
	} else {
		cmd_error("unknown method");
	}
}

/*
	cmd format: pin=N,val=V
		N can be in interval 2..9,
		V can be 1 or 0
*/
void set_cmd(char *cmd) {
	if (strlen(cmd) != 11)
		cmd_error("pin=N&val=V");
		dprint("CMD: ");dprintln(cmd);

	uint8_t num = *(cmd+4) - '0';
	uint8_t val = *(cmd+10) - '0';

	if (val == 0)
		digitalWrite(num, LOW);
	else
		digitalWrite(num, HIGH);

	cmd_ok("done");
}

const char* HTTP_OK = "HTTP/1.1 200 OK\r\n";
const char* HTTP_BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\n";
const char* HTTP_COMMON_HEADERS = "Content-Type: text/html\r\nConnection: close\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
const char* HTML_END = "</html>";
const char* HTML_BR = "<br/>";

void cmd_error(char *msg) {
	BufferString str((char*)wifi.get_buffer(), WIFI_BUFFER_SIZE);
	str.print(HTTP_BAD_REQUEST);
	str.print(HTTP_COMMON_HEADERS);
	str.print("usage: ");
	str.print(msg);
	str.print(HTML_END);

	send_reply(str.c_str());
}

void cmd_ok(char *msg) {
	BufferString str((char*)wifi.get_buffer(), WIFI_BUFFER_SIZE);
	
	str.print(HTTP_OK);
	str.print(HTTP_COMMON_HEADERS);
	str.print(msg);
	str.print(HTML_END);

	send_reply(str.c_str());
}

void reply_sensor_data() {
	BufferString str((char*)wifi.get_buffer(), WIFI_BUFFER_SIZE);
	
	str.print(HTTP_OK);
	str.print(HTTP_COMMON_HEADERS);
	for(uint8_t analogPin = 0; analogPin < 6; ++analogPin) {
		int sensorReading = analogRead(analogPin);
		str.print("analog input ");
		str.print(int_to_str(analogPin));
		str.print(" is ");
		str.print(int_to_str(sensorReading));
		str.print(HTML_BR);

	}
	for (uint8_t digitalPin = 2; digitalPin <= 9; ++digitalPin) {
		int digitalState = digitalRead(digitalPin);
		str.print("digital state ");
		str.print(int_to_str(digitalPin));
		str.print(" is ");
		str.print(int_to_str(digitalState));
		str.print(HTML_BR);
	}
	str.print(HTML_END);

	send_reply(str.c_str());
}

void send_reply(const char *str) {
	dprint("REPLY: ");
	dprintln(str);
	wifi.Reply(str);
}

