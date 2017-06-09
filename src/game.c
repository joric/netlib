#include <stdio.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#define main

#include "link.c"
#include "stun.c"
#include "bt_http.c"

#undef main

BT bt;
STUN stun;

int netOpen(char *info_hash)
{
    net_init();

//  bt_init(&bt, "http://tracker.openbittorrent.com/announce", info_hash, 655, 0);
    stun_init(&stun, "stun.xten.com", 3478, 0);

    return 0;
}

int netSend(char *buf, int len)
{
    return len;
}

int netRecv(char *buf, int buf_size)
{
    stun_update(&stun);
//  bt_update(&bt);

    return 0;
}

int netClose()
{
    net_close();
    return 0;
}

#ifndef main

int main(int argc, char **argv)
{
    const int buf_size = 4096;
    char buf[buf_size];

    netOpen("666");

    while (1)
    {
        netRecv(buf, buf_size);

        printf(".");
        Sleep(40);
    }

    netClose();

    return 0;
}

#endif
