#include "BufferString.h"

bool BufferString::print(const char *str) {
	unsigned int str_len = strlen(str);
	
	if (_size < _len + str_len + 2)
		return false;
	
	for(unsigned int i = 0; i <= str_len; i++) { // <= to include ending '\0'
		_buffer[_len + i] = str[i];
	}
	_len += str_len;

	return true;
}

bool BufferString::println(const char *str) {
	print(str);
	print("\n");
}

char *BufferString::c_str() {
	return _buffer;
}
char *BufferString::reset() {
	_len = 0;
	_buffer[0] = '\0';
	return _buffer;
}

