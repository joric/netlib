//public domain

#include "link.c"

typedef struct {
	LINK link;
	char req[4096];
	char buf[16484];
	int state;
} HTTP;

int http_init(HTTP * http, char *url)
{
	if (!url || strlen(url) + 1 >= 4096)
		return -1;

	char host[4096] = { 0 };
	char page[4096] = { 0 };
	int port = 80;

	printf("http_init: %s\n", url);

	if (sscanf(url, "http://%[^:]:%d/%s", host, &port, page) < 2)
		sscanf(url, "http://%[^/]/%s", host, page);

	sprintf(http->req, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", page, host);
	net_resolve_address(&http->link.addr, host, port);
	http->state = 1;
	http->buf[0] = 0;

	return 0;
}

int http_close(HTTP * http)
{
	return closesocket(http->link.sock);
}

int http_update(HTTP * http)
{
	int len = 0;
	LINK *link = &http->link;
	char *buf = http->buf;
	int buf_size = sizeof(http->buf);

	switch (http->state)
	{
		case 1:
			if (link->sock)
				closesocket(link->sock);

			if ((link->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				return -1;

			net_set_nonblock(link);

			//connect should be called once and will run in the background
			connect(link->sock, (struct sockaddr *)&link->addr, sizeof(ADDRESS));

			http->state = 2;

			break;

		case 2:
			if (send(link->sock, http->req, strlen(http->req) + 1, 0) > 0)
				http->state = 3;
		case 3:
			if ((len = recv(link->sock, buf, buf_size - 1, 0)) > 0)
			{
				buf[len] = 0;
				char *p = strstr(buf, "\r\n\r\n");

				if (p)
					memmove(buf, p + 4, len - (p - buf));

				http_close(http);

				http->state = 4;
			}
			break;
	}
	return (len <= 0);
}

char *urlencode(char *dest, char *src)
{
	unsigned char *s = src, *d = dest, c = 0;
	while (c = *s++)
	{
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
			*d++ = c;
		else if (*s == ' ')
			*d++ = '+';
		else
			sprintf(d, "%%%02x", c), d += 3;
	}
	*d = '\0';
	return dest;
}

#ifndef main

int main(int argc, char **argv)
{
	HTTP http;

	net_init();
	http_init(&http, "http://example.com");

	while (http_update(&http))
	{
		printf(".");
		Sleep(1);
	}

	printf(http.buf);
	return 0;
}

#endif
