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
#define WIFI_DEBUG_RX_PIN 11
#define WIFI_DEBUG_TX_PIN 12

#define WIFI_BAUDS 115200
#define DBG_BAUDS 19200
#define WIFI_BUFFER_SIZE 512

#ifdef WIFI_DEBUG

#ifdef MEGA
#define DEBUG_SERIAL Serial
#else
#include <SoftwareSerial.h>
extern SoftwareSerial dbgSerial;
#define DEBUG_SERIAL dbgSerial
#endif // MEGA

#define dprint(args...) DEBUG_SERIAL.print(args)
#define dprintln(args...) DEBUG_SERIAL.println(args)

#else // WIFI_DEBUG

#define dprint(...)
#define dprintln(...)

#endif // WIFI_DEBUG

const char AT_REPLY_OK[] = "OK";
const char AT_REPLY_READY[] = "ready";
const char AT_REPLY_NO_CHANGE[] = "no change";

const uint8_t AT_MODE_STA = 1;
const uint8_t AT_MODE_AP = 2;
const uint8_t AT_MODE_BOTH = 3;

class ESP8266 {
public:
	ESP8266(HardwareSerial& serial = Serial, const unsigned long bauds = WIFI_BAUDS):
		_serial(serial),
		_bauds(bauds)
	{
	}
	void set_reset_pin(const uint8_t pin);
	const bool begin();
	const bool set_mode(uint8_t mode);
	const bool connect(const char *ssid, const char *password);
	const char* getIP();
	const bool reset();
	const bool confServer(const uint8_t mode = 1, const uint16_t port = 80);
	const bool confMux(const bool val);
	const char* ReceiveMessage();
	bool Reply(const char *reply);
	const char* get_buffer();
private:
	HardwareSerial& _serial;
	const unsigned long _bauds;
	uint8_t _hw_reset_pin = -1;
	unsigned long _timeout = 3000;

	const void send(uint8_t num, ...);
	const void send(const char *AT_Command);
	const void sendln();
	const bool sendAndWait(const char *AT_Command, const char *AT_Response);
	const bool sendAndWait(const char *AT_Command, const char *AT_Response, const unsigned long timeout);
	void read_all();
	const int timedRead(unsigned long timeout);
	void wait_for_data(const unsigned long timeout);
	const char* receive(unsigned long timeout = 0);
	const char* receive_until(const char *endstr, unsigned long timeout = 0);
	const bool waitResponse(const char *AT_Response);
	const bool waitResponse(const char *AT_Response, const unsigned long timeout);
	const char* sendAndGetResult(const char *AT_Command, const unsigned long timeout);
	void closeMux();
};

#endif /* ESP8266_H */

