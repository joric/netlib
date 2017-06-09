//public domain

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

void setnonblock(SOCKET sock)
{
	unsigned long yes = 1;
#ifdef _WIN32
	ioctlsocket(sock, FIONBIO, &yes);
#else
	fcntl(sock, F_SETFL, O_NONBLOCK, yes);
#endif
}

int main(int argc, char **argv)
{
	SOCKET sock;
	ADDRESS addr;
    int addr_size = sizeof(ADDRESS);

	int port = 0;//30001;

    int intval = 0;

#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		printf("failed to create socket!\n");
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons((unsigned short)port);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(ADDRESS)) < 0)
	{
		printf("failed to bind socket\n");
	}
    else
    {
        getsockname(sock, (struct sockaddr *)&addr, &addr_size);
        printf("bind to: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }

    
	int yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *)&yes, sizeof(int)) < 0)
	{
		printf("failed to set socket to broadcast\n");
	}
    
	setnonblock(sock);

	while (1)
	{
        const int buf_size = 8192;
        char buf[buf_size];

        ADDRESS addr1;            
		int addr_size = sizeof(ADDRESS);

		int len = recvfrom(sock, buf, buf_size, 0, (struct sockaddr *)&addr1, &addr_size);

		if (len > 0)
        {
			printf("[recv %d bytes from %s:%d]\n", len, inet_ntoa(addr1.sin_addr), ntohs(addr1.sin_port));

            //send reply

            printf("[send %d bytes to %s:%d]\n", len, inet_ntoa(addr1.sin_addr), ntohs(addr1.sin_port));

            sprintf(buf,"hello, %s:%d, I am %s:%d\n", 
            inet_ntoa(addr1.sin_addr), ntohs(addr1.sin_port),
            inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)
            );

            len = strlen(buf);

        	sendto(sock, buf, len, 0, (struct sockaddr *)&addr1, sizeof(ADDRESS));
        }

		printf(".");
		Sleep(40);
	}

	return 0;
}

