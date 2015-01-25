#include "ESP8266Funcs.h"


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


unsigned int parse_uint(const char *start, const char *end /*=NULL*/) {
	unsigned int result = 0;
	char *p = (char *)start;
	while(p != end && *p >= '0' && *p <= '9') {
		result *= 10;
		result += *p - '0';
		p++;
	}
	return result;
}

char *int_to_str(const int i) {
	static char _buf[10];
	sprintf(_buf, "%d", i);
	return _buf;
}
