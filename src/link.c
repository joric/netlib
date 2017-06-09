//public domain

#ifndef __NET_H__
#define __NET_H__

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#define SOCKET int
#define closesocket close
#define Sleep(t) usleep(t*1000)
#endif

#define ADDRESS struct sockaddr_in

int net_millis()
{
#ifdef _WIN32
	FILETIME ft;
	LARGE_INTEGER li;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	unsigned long int ret = li.QuadPart;
	ret -= 116444736000000000LL;
	ret /= 10000;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long int ret = tv.tv_usec;
	ret /= 1000;
	ret += (tv.tv_sec * 1000);
#endif
	return ret;
}

typedef struct {
	ADDRESS addr;
	SOCKET sock;
	int port;		//local port
	int state;
} LINK;

char *net_to_string(ADDRESS * addr)
{
	static char msg[64];
	unsigned char *ip = (unsigned char *)&addr->sin_addr.s_addr;
	sprintf(msg, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], ntohs(addr->sin_port));
	return msg;
}

void net_make_address(ADDRESS * addr, unsigned int ip, int port)
{
	memset(addr, 0, sizeof(ADDRESS));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = ip;
}

int net_resolve_address(ADDRESS * addr, char *host, int port)
{
	printf("resolving %s\n", host);

	int t = net_millis();

	net_make_address(addr, 0, port);
	struct hostent *hp = (struct hostent *)gethostbyname(host);

	if (hp)
		*(int *)&addr->sin_addr.s_addr = *(int *)hp->h_addr_list[0];
	else
		addr->sin_addr.s_addr = inet_addr(host);

	printf("resolved to %s (%d ms)\n", net_to_string(addr), net_millis() - t);
}

int net_is_local_address(ADDRESS * addr)
{
	SOCKET sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ADDRESS loc_addr;
	net_make_address(&loc_addr, addr->sin_addr.s_addr, 0);	//ephemeral
	printf("checking NAT: binding to %s ", net_to_string(&loc_addr));
	int res = (bind(sock, (struct sockaddr *)&loc_addr, sizeof(ADDRESS)) == 0);
	closesocket(sock);
	printf(res ? "success, no NAT\n" : "failed, NAT present\n");
	return res;
}

int net_local_port(LINK * link)
{
	link->port = 0;		//ephemeral
	ADDRESS loc_addr;
	net_make_address(&loc_addr, INADDR_ANY, link->port);

	if (bind(link->sock, (struct sockaddr *)&loc_addr, sizeof(ADDRESS)) == 0)
	{
		int addr_size = sizeof(ADDRESS);
		getsockname(link->sock, (struct sockaddr *)&loc_addr, &addr_size);
		printf("local port: %d\n", loc_addr.sin_port);
		return loc_addr.sin_port;
	}
	return 0;
}

void net_set_nonblock(SOCKET sock)
{
	unsigned long nonblock = 1;
#ifdef _WIN32
	ioctlsocket(sock, FIONBIO, &nonblock);
#else
	fcntl(sock, F_SETFL, O_NONBLOCK, nonblock);
#endif
}

int net_send(LINK * link, char *buf, int len)
{
	printf("[send %d bytes to %s]\n", len, net_to_string(&link->addr));
	sendto(link->sock, buf, len, 0, (struct sockaddr *)&link->addr, sizeof(ADDRESS));
}

int net_recv(LINK * link, char *buf, int buf_size)
{
	ADDRESS addr;
	int addr_size = sizeof(ADDRESS);
	int len = recvfrom(link->sock, buf, buf_size, 0, (struct sockaddr *)&addr, &addr_size);
	if (len > 0)
		printf("[recv %d bytes from %s]\n", len, net_to_string(&addr));

	return len;
}

int net_open_link(LINK * link, char *host, int port)
{
	link->state = 0;
	net_resolve_address(&link->addr, host, port);
//      printf("resolved to %s\n", net_to_string(&link->addr));

	if ((link->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		goto cleanup;
	net_set_nonblock(link->sock);
	link->port = net_local_port(link);
	link->state = 1;
	return 0;

cleanup:
	closesocket(link->sock);
	return 1;
}

int net_init()
{
#ifdef _WIN32
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
#endif
	return 0;
}

int net_close()
{
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

int net_set_address(LINK * link, ADDRESS * addr)
{
	memcpy(&link->addr, addr, sizeof(ADDRESS));
}

#endif //__NET_H__


