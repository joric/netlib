#ifndef UTILS_H
#define UTILS_H


namespace net {

	void RandomKey(char *buf, int from, int to, int len)
	{
		int i;
		for (i = 0; i < len; i++)
			buf[i] = from + (rand() % (to - from));
	};

	void DumpHex(unsigned char *buf, int len)
	{
		int i = 0;
		for (i = 0; i < len; i++)
			printf("%02x ", (unsigned char) buf[i]);
		printf("\n");
	}

}

#endif