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
	int sock;
    struct sockaddr_in addr;

    int addr_size = sizeof(addr);

	int port = 27960;

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


	int enable = 1;

	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *)&enable, sizeof(enable)) < 0)
	{
		printf("failed to set socket to broadcast\n");
	}

	setnonblock(sock);

	while (1)
	{
        const int buf_size = 8192;
        char buf[buf_size];

        struct sockaddr_in addr1;            

		int addr_size = sizeof(addr);

        int len = 0;
        sprintf(buf, "getinfo");
        len = strlen(buf);

        addr1.sin_family = AF_INET;
//        addr1.sin_addr.s_addr = inet_addr("224.0.0.251");
        addr1.sin_addr.s_addr = htonl(INADDR_BROADCAST);

//        addr1.sin_addr.s_addr = inet_addr("10.1.1.1");

        addr1.sin_port = htons(port);

        printf("sending query\n");

        sendto(sock, buf, len, 0, (struct sockaddr *)&addr1, sizeof(addr));
      
		len = recvfrom(sock, buf, buf_size, 0, (struct sockaddr *)&addr1, &addr_size);

		if (len > 0)
        {
			printf("[recv %d bytes from %s]\n", len, inet_ntoa(addr1.sin_addr));

            buf[len] = 0;

            printf("%s\n", buf);

            /*
            //send reply

            printf("[send %d bytes to %s]\n", len, inet_ntoa(addr1.sin_addr));

            sprintf(buf,"hello, %s:%d, I am %s:%d\n", 
            inet_ntoa(addr1.sin_addr), ntohs(addr1.sin_port),
            inet_ntoa(addr.sin_addr), ntohs(addr.sin_port)            
            );
            */

            len = strlen(buf);

        	sendto(sock, buf, len, 0, (struct sockaddr *)&addr1, sizeof(addr));
        }

		Sleep(1000);
	}


	return 0;
}

