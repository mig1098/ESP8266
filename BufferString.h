#ifndef BUFFERSTRING_H
#define BUFFERSTRING_H

#include <string.h>

class BufferString {
	public:
		BufferString(char *buffer, const unsigned int size):
			_buffer(buffer),
			_size(size)
		{
			*_buffer = '\0';
		}
		bool print(const char *str);
		bool println(const char *str);
		char *c_str();
		char *reset();
	private:
		int _len = 0;
		unsigned int _size;
		char *_buffer; 
};

#endif // BUFFERSTRING
