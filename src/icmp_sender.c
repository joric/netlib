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

int main(int argc, char **argv)
{
    char *host = "127.0.0.1";

    if (argc > 1)
        host = argv[1];

#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = inet_addr(host);

    //??? can't send directly to 10.1.1.8, only to the 10.1.1.1

    char buf[256];
    int len = 1;

    char b[] = "\x08\x00\xf0\xb8\x07\x43\x00\x04";

    len = sizeof(b) - 1;

    memcpy(buf, b, len);

    while (1)
    {
        printf("send %d byte(s) to %s\n", len, inet_ntoa(addr.sin_addr));
        sendto(sock, buf, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));
        Sleep(1000);
    }

}
