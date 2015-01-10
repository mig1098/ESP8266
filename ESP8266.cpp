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

#include "ESP8266.h"
//#include "Dns.h"
/* Helpers */

// TODO: maybe it is better to use inet_aton from Dns.h,
// but calling that function requires creating Dns object
const bool verifyIP(const char *str ) {
	char *p = (char *)str;
	uint8_t dots = 0;
	while (*p) {
		if (*p == '.')
			++dots;
		else if ((*p >= '0') || (*p <= '9') )
			; //ok
		else
			return false;
	}
	if (dots != 3)
		return false;

	return true;
}

/* class methods */

void ESP8266::set_reset_pin(const uint8_t pin) {
	_hw_reset_pin = pin;
	if (_hw_reset_pin > 0)
		digitalWrite(_hw_reset_pin, HIGH);
}

const bool ESP8266::begin() {
	_serial.begin(_bauds);
	_serial.setTimeout(100);

	// basic test
	if(!sendAndWait(AT, AT_REPLY_OK, 1))
		return false;

	if(!reset())
		return false;

	set_mode(AT_MODE_BOTH);
	
	if(!reset())
		return false;

	return true;
}

void ESP8266::read_all() {
	while(_serial.available())
		_serial.read();
}

void ESP8266::set_mode(const char *mode) {
	send(2, AT_MODE, mode);
	read_all();
}

const bool ESP8266::connect(const char *ssid, const char *password) {
	send(5, "AT+CWJAP=\"", ssid, "\",\"", password, "\"");
	
	if(!waitResponse(AT_REPLY_OK))
		return false;

	delay(5000);

	unsigned long start_time = millis();
	while(!verifyIP(getIP())) {
		if (millis() - start_time > 30000)
			return false;
	}

	return true;
}

const char *ESP8266::getIP() {
	static char ip[16];
	memset(ip, 0, sizeof(ip));
	char *str = (char *)sendAndGetResult(AT_CHECK_IP, 1000);

	uint8_t n = 0; // count \n
	while(*str && n < 2)
		if(*str == '\n') ++str;

	if (n != 2)
		return ip; // empty
	++str;
	n = 0; // now it is for addressing
	while(*str) {
		if(*str == '\n')
			break;
		ip[n] = *str;
		++n;
		++str;
	}

	return ip;
}


const bool ESP8266::reset() {
	uint8_t reset_retries = 10;
	if (_hw_reset_pin > 0)
		reset_retries = 5;

	// Try software reset
	for (uint8_t retry = 0; retry < reset_retries; ++retry) {
		if(sendAndWait(AT_RESTART, AT_REPLY_READY))
			return true;
	}

	// Try hardware reset if it is possible
	for (uint8_t retry = 0; retry < reset_retries; ++retry) {
		if (_hw_reset_pin > 0) {
			digitalWrite(_hw_reset_pin, LOW);
			delay(700);
			digitalWrite(_hw_reset_pin, HIGH);
		}
		if(waitResponse(AT_REPLY_READY)) {
			return true;
		}
	}

	return false;
}

const bool ESP8266::waitResponse(const char *AT_Response) {
	return waitResponse(AT_Response, _timeout);
}

const bool ESP8266::waitResponse(const char *AT_Response, const unsigned long timeout) {
	unsigned long start_time = millis();

	while (1) {
		if (_serial.available() && _serial.find((char*)AT_Response))
		{
			return true;
		}

		if (millis() - start_time > timeout)
			break;
		delay(100); // Try to be energy efficient and don't use spinlocks
	}

	return false;
	
}


const bool ESP8266::sendAndWait(const char *AT_Command, const char *AT_Response) {
	return sendAndWait(AT_Command, AT_Response, _timeout);
}

const bool ESP8266::sendAndWait(const char *AT_Command, const char *AT_Response, const unsigned long timeout) {
	send(AT_Command);

	return waitResponse(AT_Response, timeout);
}

const void ESP8266::send(uint8_t num, ...) {
	va_list argList;
	va_start(argList,num);
	for(; num; num--) {
		char *str = va_arg(argList, char*);
		Serial.print(str);
		_serial.print(str);
	}
	va_end(argList);
	Serial.println("");
	_serial.println("");
}

const void ESP8266::send(const char *AT_Command) {
	send(1, AT_Command);
}

const char* ESP8266::receive() {
	static char buffer[WIFI_READ_BUFFER_SIZE];

	uint8_t n = 0;
	memset(buffer, 0, sizeof(buffer));
	while(_serial.available() && n < WIFI_READ_BUFFER_SIZE - 1) {
		buffer[n] = _serial.read();
		n++;
	}
	buffer[n] = '\0';

	return buffer;
}

const char* ESP8266::sendAndGetResult(const char *AT_Command, const unsigned long timeout) {
	Serial.print("Sending "); Serial.println(AT_Command);

	send(AT_Command);

	unsigned long start_time = millis();
	while(1) {
		Serial.println("loop");
		if (_serial.available())
			break;
		Serial.println("check1");
		if(millis() - start_time > timeout)
			break;

		Serial.print("now: "); Serial.print(millis()); Serial.print("start:"); Serial.println(start_time);
		delay(100); // Try to be energy efficient and don't use spinlocks
	}
		
	return receive();
}

