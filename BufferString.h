#ifndef BUFFERSTRING_H
#define BUFFERSTRING_H

#include <string.h>

class BufferString {
	public:
		BufferString(char *buffer, const unsigned int size):
			_buffer(buffer),
			_size(size)
		{
			reset();
		}
		bool print(const char *str);
		bool println(const char *str);
		char *c_str();
		char *reset();
	private:
		int _len;
		unsigned int _size;
		char *_buffer; 
};

#endif // BUFFERSTRING
