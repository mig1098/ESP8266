/*
* (C) Copyright 2014 giannoug. All rights reserved.
*
* This file is part of the ESP8266 library for the Arduino (https://github.com/giannoug/ESP8266)
*
* The ESP8266 library is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef ESP8266_H
#define ESP8266_H

#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

//#define MEGA
#define WIFI_DEBUG

#define WIFI_BAUDS 115200
#define WIFI_READ_BUFFER_SIZE 128

#ifdef WIFI_DEBUG

#ifdef MEGA
#define DEBUG_SERIAL Serial
#else
#include <SoftwareSerial.h>
extern SoftwareSerial dbgSerial;
#define DEBUG_SERIAL dbgSerial
#endif // MEGA
//	#define DBG(num, args...) print_to_stream(DEBUG_SERIAL, args)
#define dprint(args...) DEBUG_SERIAL.print(args)
#define dprintln(args...) DEBUG_SERIAL.println(args)

#else // WIFI_DEBUG

#define dprint(...)
#define dprintln(...)

#endif // WIFI_DEBUG

const char AT[] = "AT";
const char AT_RESTART[] = "AT+RST";
const char AT_MODE[]    = "AT+CWMODE=";
const char AT_JOIN_AP[] = "AT+CWJAP=\"{0}\",\"{1}\"";
//const char AT_LIST_AP[] = "AT+CWLAP";
//const char AT_QUIT_AP[] = "AT+CWQAP";
//const char AT_AP_MODE[] = "AT+CWSAP";
const char AT_CHECK_IP[] = "AT+CIFSR";

const char AT_REPLY_OK[] = "OK";
const char AT_REPLY_READY[] = "ready";

const char AT_MODE_STA[] = "1";
const char AT_MODE_AP[] = "2";
const char AT_MODE_BOTH[] = "3";

class ESP8266 {
public:
	ESP8266(HardwareSerial& serial = Serial, const unsigned long bauds = WIFI_BAUDS):
		_serial(serial),
		_bauds(bauds)
	{
	}
	void set_reset_pin(const uint8_t pin);
	const bool begin();
	void set_mode(const char *mode);
	const bool connect(const char *ssid, const char *password);
	const char* getIP();
	const bool reset();
private:
	HardwareSerial& _serial;
	const unsigned long _bauds;
	uint8_t _hw_reset_pin = -1;
	unsigned long _timeout = 1000;

	const void send(uint8_t num, ...);
	const void send(const char *AT_Command);
	const bool sendAndWait(const char *AT_Command, const char *AT_Response);
	const bool sendAndWait(const char *AT_Command, const char *AT_Response, const unsigned long timeout);
	void read_all();
	const int timedRead(unsigned long timeout);
	const char* receive(unsigned long timeout = 0);
	const bool waitResponse(const char *AT_Response);
	const bool waitResponse(const char *AT_Response, const unsigned long timeout);
	const char* sendAndGetResult(const char *AT_Command, const unsigned long timeout);
};

#endif /* ESP8266_H */

