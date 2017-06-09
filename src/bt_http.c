//public domain

#include <stdio.h>

#define main

#include "http.c"
#include "utils.c"

#undef main

typedef struct {
	HTTP http;
	ADDRESS peer[200];
	char announce[256];
	int peers;
	int state;
	ADDRESS addr;
	int send_time;
} BT;

int bt_init(BT * bt, char *announce, char *info_hash, int port, int event)
{
	char buf[4096];
	char str[256];
    char hash[21] = {0};
	static char *events[] = { "", "completed", "started", "stopped" };
	char peer_id[] = "-UT2000-001234567890";
	strcpy(bt->announce, announce);

    atoh(hash, info_hash, 20);

   	sprintf(buf, "%s?info_hash=%s", announce, urlencode(str, hash));
	sprintf(buf, "%s&peer_id=%s", buf, urlencode(str, peer_id));
//    sprintf(buf, "%s&ip=%s", buf, "207.46.197.32");
	sprintf(buf, "%s&port=%d", buf, port);
    sprintf(buf, "%s&uploaded=0&downloaded=0&left=0", buf);
	sprintf(buf, "%s&event=%s", buf, events[event]);
    sprintf(buf, "%s&compact=%d", buf, 1);

	bt->state = 1;
	bt->peers = 0;
	bt->send_time = time(0);

	http_init(&bt->http, buf);
}

void bt_get_peers(BT * bt)
{
	int peers = 0;
	char *p = strstr(bt->http.buf, ":peers");
	if (p)
	{
		int i;
		sscanf(p, ":peers%d:", &i);
		peers = i / 6;
		p = strstr(p + 1, ":") + 1;
        if (peers > 0 && peers <= 200)
		{
			for (i = 0; i < peers; i++)
			{
				int ip = *(int *)p;
				p += 4;
				short port = ntohs(*(short *)p);
				p += 2;
				net_make_address(&bt->peer[i], ip, port);
				printf("peer %d: %s\n", i, net_to_string(&bt->peer[i]));
			}
			bt->peers = peers;
		}
	}
}

int bt_update(BT * bt)
{
	int res = 0;
	switch (bt->state)
	{
		case 1:
			res = http_update(&bt->http);
			if (res == 0)
			{
				printf("we got a response (%d bytes)\n", strlen(bt->http.buf));

				bt_get_peers(bt);
				bt->state = 1;
			}
			break;
	}
	return res;
}

#ifndef main

int main(int argc, char **argv)
{
	net_init();
	BT bt;

    char* info_hash = "8241bdf1f1214014297f87c93ee58a57a983ccc4";

    bt_init(&bt, "http://tracker.openbittorrent.com:80/announce", info_hash, 666, 2);

	while ( bt_update(&bt) )
	{
		printf(".");
		Sleep(40);
	}

	return 0;
}

#endif
