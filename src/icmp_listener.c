#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#define closesocket close
#define Sleep(t) usleep(t*1000)
#endif

unsigned short calc_icmp_checksum(unsigned short *data, int bytes)
{
	unsigned int sum;
	int i;

	sum = 0;
	for (i = 0; i < bytes / 2; i++)
	{
		sum += data[i];
	}
	sum = (sum & 0xFFFF) + (sum >> 16);
	sum = htons(0xFFFF - sum);
	return sum;
}

void icmp_parse(char *buf, int len)
{
	int i;

	char ihl = *(char *)(buf + 0);
	char tos = *(char *)(buf + 1);
	short ln = ntohs(*(short *)(buf + 2));
    unsigned char ttl = *(unsigned char *)(buf + 8);
	char prot = *(char *)(buf + 9);
	char type = *(char *)(buf + 20);
	char code = *(char *)(buf + 21);
	unsigned short crc = ntohs(*(short *)(buf + 22));
	unsigned int quench = ntohl(*(long *)(buf + 24));

	char p[1024];
	int icmp_len = len - 20;
	char *icmp_buf = buf + 20;
	memcpy(p, icmp_buf, icmp_len);

	for (i = 0; i < icmp_len; i++)
	{
		printf("\\x%02x", (unsigned char)p[i]);
	}

	printf("\n");

	p[2] = 0;
	p[3] = 0;

	unsigned short crc2 = calc_icmp_checksum((unsigned short *)p, icmp_len);

	printf("len: %d ttl: %d type: %d code: %d crc: %u quench: %u crc2: %u\n", ln, ttl, type, code, crc, quench, crc2);




	if (len <= 28)
		return;

	for (i = 28; i < len; i++)
	{
		unsigned char c = buf[i];
		printf("%c", c >= 32 && c <= 127 ? c : '.');
	}
	printf("\n");
}


int main(int argc, char **argv)
{
	int i;

#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	const int buf_size = 256;
	char buf[buf_size];
	int len = 1;

#ifdef _WIN32
	//we can't use 127.0.0.1 in promiscuous mode so we have to determine host address
	unsigned int val = 1;
	DWORD bytes;
	struct hostent *h;

	if ((gethostname(buf, buf_size)) == -1)
		printf("unable to gethostname\n");

	if ((h = gethostbyname(buf)) == NULL)
		printf("unable to gethostbyname\n");

	*(int *)&addr.sin_addr.s_addr = *(int *)h->h_addr_list[0];

	if ((bind(sock, (struct sockaddr *)&addr, sizeof(addr))) == -1)
		printf("unable to bind() socket\n");

	if ((WSAIoctl(sock, _WSAIOW(IOC_VENDOR, 1), &val, sizeof(val), NULL, 0, &bytes, NULL, NULL)) == -1)
		printf("failed to set promiscuous mode\n");
#endif

	printf("listening on %s...\n", inet_ntoa(addr.sin_addr));

	int counter = 0;

	while (1)
	{
		struct sockaddr_in addr;
		int addr_size = sizeof(addr);

		len = recvfrom(sock, buf, buf_size, 0, (struct sockaddr *)&addr, &addr_size);

		if (len > 0)
		{
			printf("%d: recv %d bytes from %s\n", counter, len, inet_ntoa(addr.sin_addr));
			icmp_parse(buf, len);
			Sleep(1);
			counter++;
		}
	}
}
