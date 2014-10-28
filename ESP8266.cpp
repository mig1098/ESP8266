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

#include "ESP8266.H"

uint8_t ESP8266::begin() {
	return sendAndWait(String("AT"), "OK", 1);
}

uint8_t ESP8266::connect(const char *ssid, const char *password) {
	String command = "AT+CWJAP=\"{0}\",\"{1}\"";

	command.replace("{0}", ssid);
	command.replace("{1}", password);

	return sendAndWait(command, "OK", 1);
}

String ESP8266::getIP() {
	return getValue(sendAndGetResult("AT+CIFSR", 1), '\n', 2);
}

uint8_t ESP8266::sendAndWait(String AT_Command, char *AT_Response, uint16_t wait) {
	serial.println(AT_Command);

	delay(wait);

	while (Serial1.available()) {
		if (serial.find(AT_Response))
		{
			return 1;
		}
	}

	return 0;
}

String ESP8266::sendAndGetResult(String AT_Command, uint16_t wait) {
	serial.println(AT_Command);

	delay(wait);

	return serial.readString();
}

/* Helpers */

// http://stackoverflow.com/a/14824108/754565
String ESP8266::getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++){
		if (data.charAt(i) == separator || i == maxIndex){
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}

	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}