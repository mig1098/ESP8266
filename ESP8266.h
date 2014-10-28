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

class ESP8266 {
public:
	ESP8266(Stream& s = Serial) :serial(s) {}
	uint8_t begin();
	uint8_t connect(const char *ssid, const char *password);
	String getIP();
private:
	Stream& serial;

	uint8_t sendAndWait(String AT_Command, char *AT_Response, uint16_t wait);
	String sendAndGetResult(String AT_Command, uint16_t wait);

	/* Helpers */
	String getValue(String data, char separator, int index);
};

#endif /* ESP8266_H */