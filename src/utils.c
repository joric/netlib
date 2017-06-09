//public domain

#ifndef __UTILS_H__
#define __UTILS_H__

char *htoa(char *dest, char *src, int len)
{
    char *d = dest;
    while( len-- )
        sprintf(d, "%02x", (unsigned char)*src++), d += 2;
    return dest;
}

void atoh(char *dest, char *src, int len)
{
	int v = 0;
	while ( sscanf(src, "%02x", &v) > 0 )
		*dest++ = v, src += 2;
}

int pad(int offset, int align)
{
	return offset + ((align - (offset % align)) % align);
}

void rndstr(char *buf, int a, int b, int len)
{
	int i;
	for (i = 0; i < len; i++)
		buf[i] = a + rand() % (b - a);
}

#endif  //__UTILS_H__

#ifndef main

#include <stdio.h>

int main(int argc, char **argv)
{
    char* str = "deadbeef";

    char val[4];
    char asc[9];

    printf("src: %s\n", str);

    atoh(val, str, 4);

    printf("res: %s\n", htoa(asc, val, 4));

	return 0;
}

#endif
