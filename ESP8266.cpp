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

#ifdef WIFI_DEBUG
SoftwareSerial dbgSerial(11, 12);
#endif

/* Helpers */

// TODO: maybe it is better to use inet_aton from Dns.h,
// but calling that function requires creating Dns object
const bool verifyIP(const char *str ) {
	char *p = (char *)str;
	uint8_t dots = 0;
	while (*p) {
		if (*p == '.')
			++dots;
		else if ((*p >= '0') && (*p <= '9') )
			; //ok
		else
			return false;
		++p;
	}
	if (dots != 3)
		return false;

	return true;
}

void print_to_stream(Stream& stream, uint8_t& num, va_list argList) {
	for(; num; num--) {
		char *str = va_arg(argList, char*);
		dprint(str);
		stream.print(str);
	}
	dprintln("");
	stream.println("");
}

/* class methods */

void ESP8266::set_reset_pin(const uint8_t pin) {
	_hw_reset_pin = pin;
	if (_hw_reset_pin > 0)
		digitalWrite(_hw_reset_pin, HIGH);
}

const bool ESP8266::begin() {
#ifdef WIFI_DEBUG
	dbgSerial.begin(9600);
#endif
	_serial.begin(_bauds);

	// basic test
	if(!sendAndWait("AT", AT_REPLY_OK))
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
	send(2, "AT+CWMODE=", mode);
	read_all();
}

const bool ESP8266::connect(const char *ssid, const char *password) {
	send(5, "AT+CWJAP=\"", ssid, "\",\"", password, "\"");
	
	if(!waitResponse(AT_REPLY_OK))
		return false;

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

	char *str = (char *)sendAndGetResult("AT+CIFSR", 3000);
	char *pstr = str;

	while(*pstr && *pstr != '.')
		++pstr;
	
	if (pstr == str)
		return ip; // empty
	
	while(*pstr != '\n' && pstr != str) {
		--pstr;
	}

	++pstr;
	uint8_t n = 0;
	while(*pstr && n < 16) {
		if(!(*pstr == '.' || (*pstr >= '0' && *pstr <= '9')))
			break;
		ip[n] = *pstr;
		++n;
		++pstr;
	}

	dprint("getIP returned: '");
	dprint(ip);
	dprintln("'");
	return ip;
}


const bool ESP8266::reset() {
	uint8_t reset_retries = 10;
	if (_hw_reset_pin > 0)
		reset_retries = 5;

	// Try software reset
	for (uint8_t retry = 0; retry < reset_retries; ++retry) {
		if(sendAndWait("AT+RST", AT_REPLY_READY))
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
	read_all();

	send(AT_Command);

	return waitResponse(AT_Response, timeout);
}

const void ESP8266::send(uint8_t num, ...) {
	va_list argList;
	va_start(argList, num);
	
	dprint("CMD: ");
	print_to_stream(_serial, num, argList);
	va_end(argList);

}

const void ESP8266::send(const char *AT_Command) {
	send(1, AT_Command);
}


// copy of protected method from Stream class
const int ESP8266::timedRead(unsigned long timeout) {
  int c;
  unsigned long _startMillis = millis();
  do {
    c = _serial.read();
    if (c >= 0) return c;
  } while(millis() - _startMillis < timeout);
  return -1;     // -1 indicates timeout
}

const char* ESP8266::receive(unsigned long timeout /*= 0*/) {
	static char buffer[WIFI_READ_BUFFER_SIZE];

	uint8_t n = 0;
	memset(buffer, 0, sizeof(buffer));

	if(timeout == 0)
		timeout = _timeout;
	while(n < WIFI_READ_BUFFER_SIZE - 1) {
		int c = timedRead(timeout);
		if (c < 0)
			break;
		buffer[n] = c;
		n++;
	}
	buffer[n] = '\0';

	dprint("GOT: ");
	dprintln(buffer);
	return buffer;
}

const char* ESP8266::sendAndGetResult(const char *AT_Command, const unsigned long timeout) {
	read_all();

	send(AT_Command);

	unsigned long start_time = millis();
	while(1) {
		if (_serial.available())
			break;
		if(millis() - start_time > timeout)
			break;

		delay(100); // Try to be energy efficient and don't use spinlocks
	}
	return receive(timeout);
}

