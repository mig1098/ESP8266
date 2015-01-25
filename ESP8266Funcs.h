#ifndef ESP8266FUNCS_H
#define ESP8266FUNCS_H

#include <Arduino.h>
#include <Stream.h>
#include <stdio.h>
#include <string.h>

const bool verifyIP(const char *str );
unsigned int parse_uint(const char *start, const char *end = NULL);
char *int_to_str(const int i);

#endif

