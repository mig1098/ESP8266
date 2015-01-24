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
#include "ESP8266Funcs.h"
//#include "Dns.h"

#ifdef WIFI_DEBUG
#ifndef MEGA
SoftwareSerial dbgSerial(WIFI_DEBUG_RX_PIN, WIFI_DEBUG_TX_PIN);
#endif
#endif

char buffer[WIFI_BUFFER_SIZE];
int8_t conn_id;

/* Helper functions */
void print_to_stream(Stream& stream, uint8_t& num, va_list argList) {
	for(; num; num--) {
		char *str = va_arg(argList, char*);
		dprint(str);
		stream.print(str);
	}
}

/* class methods */

void ESP8266::set_reset_pin(const uint8_t pin) {
	_hw_reset_pin = pin;
	if (_hw_reset_pin > 0)
		digitalWrite(_hw_reset_pin, HIGH);
}

const bool ESP8266::begin() {
#ifdef WIFI_DEBUG
	DEBUG_SERIAL.begin(DBG_BAUDS);
#endif
	_serial.begin(_bauds);

	if(!reset())
		return false;

	// basic test
	if(!sendAndWait("AT", AT_REPLY_OK))
		return false;

	set_mode(AT_MODE_BOTH);
	
	return true;
}

void ESP8266::read_all() {
	while(_serial.available())
		_serial.read();
}

const bool ESP8266::set_mode(uint8_t mode) {
	sprintf(buffer, "AT+CWMODE=%u", mode);
	
	char *reply = (char *)sendAndGetResult(buffer);

	if(strstr(buffer, AT_REPLY_NO_CHANGE))
		return true;

	if (strstr(buffer, AT_REPLY_OK))
		return reset();
	
	return false;
}

const bool ESP8266::connect(const char *ssid, const char *password) {
	send(5, "AT+CWJAP=\"", ssid, "\",\"", password, "\"");
	sendln();
	
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

	char *str = (char *)sendAndGetResult("AT+CIFSR");
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

	return ip;
}


const bool ESP8266::reset() {
	uint8_t reset_retries = 10;
	if (_hw_reset_pin > 0)
		reset_retries = 5;

	// Try software reset
	for (uint8_t retry = 0; retry < reset_retries; ++retry) {
		dprint("rst:"); dprintln(retry);
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

	dprintln("wr");
	char *reply = (char *)receive_until(AT_Response, timeout);
	dprintln("ru fin");
	return strstr(reply, AT_Response);
}


const bool ESP8266::sendAndWait(const char *AT_Command, const char *AT_Response) {
	return sendAndWait(AT_Command, AT_Response, _timeout);
}

const bool ESP8266::sendAndWait(const char *AT_Command, const char *AT_Response, const unsigned long timeout) {
	read_all();

	send(AT_Command);
	sendln();

	return waitResponse(AT_Response, timeout);
}

const void ESP8266::sendln() {

	_serial.println("");
	dprintln("");

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

const bool ESP8266::wait_symbol(const char symbol, const unsigned long timeout) {
	int c;
	unsigned long _start = millis();

	while(millis() - _start < timeout) {
		c = _serial.read();
		if (c > 0 && c == symbol)
			return true;
	}

	return false;
}

const char* ESP8266::receive_until(const char *endstr, unsigned long timeout /*= 0*/) {
	unsigned long read_timeout = timeout;

	if(read_timeout == 0)
		read_timeout = _timeout;

	if(endstr) {
		read_timeout = 300;
//		dprint("endstr: ");
//		dprintln(endstr);
	}

//	dprint("timeout=");
//	dprint(timeout);
//	dprint(", read_timeout=");
//	dprintln(read_timeout);

	unsigned long start_time = millis();
	uint16_t n = 0;
	while(n < WIFI_BUFFER_SIZE - 1) {
		int c = timedRead(read_timeout);
		if (c < 0) {
			// stop reading if we don't wait for specific ending
			// or if timeout exceded
			if(!endstr || strstr(buffer, endstr))
				break;
			// else, break if endstr found
			if(millis() - start_time > timeout)
				break;
		} else {
			buffer[n] = c;
			n++;
		}
	}
	buffer[n] = '\0';

	dprint("GOT: '");
	dprint(buffer);
	dprintln("'");

	return buffer;
}

const char* ESP8266::receive(unsigned long timeout /*= 0*/) {
	return receive_until(NULL, timeout);
}

void ESP8266::wait_for_data(unsigned long timeout) {
	if(timeout == 0)
		timeout = _timeout;
	unsigned long start_time = millis();
	while(1) {
		if (_serial.available())
			break;
		if(millis() - start_time > timeout)
			break;

		delay(100); // Try to be energy efficient and don't use spinlocks
	}
}

const char* ESP8266::sendAndGetResult(const char *AT_Command, const unsigned long timeout) {
	read_all();

	send(AT_Command);
	sendln();

	wait_for_data(timeout);

	return receive();
}

const bool ESP8266::confMux(const bool val) {
	sprintf(buffer, "AT+CIPMUX=%u", val ? 1 : 0);

	return sendAndWait(buffer, AT_REPLY_OK);
}

const bool ESP8266::confServer(const uint8_t mode /* = 1 */, const uint16_t port /* = 80 */) {
	confMux(true);

	sprintf(buffer, "AT+CIPSERVER=%u,%u", mode, port);

	char *reply = (char *)sendAndGetResult(buffer);
	
	if (strstr(buffer, AT_REPLY_OK) || strstr(buffer, AT_REPLY_NO_CHANGE))
		return true;

	return false;
}

const char* ESP8266::ReceiveMessage() {
	//+IPD,<len>:<data>
	//+IPD,<id>,<len>:<data>
	char *msg = (char*)receive_until("\nOK", 5000);

	if (!strlen(msg))
		return NULL;

	char *msgp = strstr(msg, "+IPD");
	if (!msgp)
		return NULL;

	msgp += 5;

	/* parse connection id if multiple connection mode */
	conn_id = -1;
	char *p = msgp;
	for(char *p = msgp; *p != ':'; ++p) {
		if(*p == ',') {
			conn_id = parse_uint(msgp);
			dprint("conn_id=");dprintln(conn_id);
			break;
		}
	}
	/* *********************************************** */
	
	msgp = strstr(msgp, ":");
	msgp++;

//	dprint("RCV: ");
//	dprintln(msgp);
	return msgp;
}


const char* ESP8266::get_buffer() {
	return buffer;
}


bool ESP8266::Reply(const char *reply) {
	char cmd[20];
	if(conn_id >=0)
		sprintf(cmd, "AT+CIPSEND=%u,%u", conn_id, strlen(reply));
	else
		sprintf(cmd, "AT+CIPSEND=%u", strlen(reply));

	send(cmd);
	sendln();

	bool result = true;
	if(!wait_symbol('>', 5000)) {
		result = false;
		dprintln("no prompt");
		goto reply_end;
	}
/*
	send("AT+CIPSEND=");
	if(conn_id >= 0) {
		send(int_to_str(conn_id));
		send(",");
	}

	send(int_to_str(strlen(reply)));
	sendln();

	bool result = true;
	unsigned long start = millis();
	// FIXME: remove this debug mess
	dprint("? ");
	char r[2];
	memset(r,0,2);
	while(1) {
		int c = timedRead(300);
		if(c > 0) {
			r[0] = c;
			dprint(r);
		}
		if (c == '>')
			break;
		if(millis() - start > 20000) {
			result = false;
			dprintln("no prompt");
			goto reply_end;
		}
	}
*/
	result = sendAndWait(reply, "SEND OK");

reply_end:
	closeMux();

	return result;
}

void ESP8266::closeMux() {
	send("AT+CIPCLOSE");
	if (conn_id >= 0) {
		send("=");
		send(int_to_str(conn_id));
	}
	sendln();

	conn_id = -1;

	receive(1000); // FIXME: very long static timeout here
	// FIXME: wait for one of
	// "Linked" or "ERROR" or "we must restart" in single mode
	// if "OK" or "Link is not" or "Cant close" in multiple connection mode
}

