//public domain

#include <stdio.h>

#define main

#include "packet.c"
#include "link.c"
#include "utils.c"

#undef main

typedef struct {
	LINK link;
	ADDRESS peer[200];
	char announce[256];
	int peers;
	int state;
	int packet_type;
	int send_time;
} BT;

enum {
	bt_connection_request = 1,
	bt_connection_response,
	bt_announce_request,
	bt_announce_response,
	bt_scrape_request,
	bt_scrape_response,
	bt_error_response
};

unsigned long long bt_connection_id;

int bt_peer_port = 666;
int bt_peer_key;
int bt_peer_event;
int bt_packet_type = 0;

char *bt_tracker = "udp://tracker.openbittorrent.com:80/announce";
char *bt_str_hash = "8241bdf1f1214014297f87c93ee58a57a983ccc4";
char bt_peer_id[] = "-UT2000-xG1268598276";
char bt_info_hash[20];

PACKET udp_ring[16];

void dump(char *buf, int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
		printf("%02x ", (unsigned char)buf[i]);
	printf("\n");
}

#define T(t, v) printf( " %s: 0x%lx\n", t, v )
#define D(t, v) printf( " %s: %ld\n", t, v )

int bt_parse_packet(char *buf, int type, int len)
{
	PACKET *m = &udp_ring[0];
	packet_init(m, buf, len);

    if (len < 20)
        return -1;

	printf("Packet type: %d\n", type);

	switch (type)
	{
		case bt_connection_response:
		{
			T("action", r32(m));
			T("transaction_id", r32(m));
			T("connection_id", bt_connection_id = r64(m));
			break;
		}

		case bt_announce_response:
		{
			int action = r32(m);

			if (action == 3)	//error
			{
				T("action", action);
				T("transaction_id", r32(m));
				printf("error: %s\n", m->buf);

			}
			else
			{
				int i;
				int seeders;
				int leechers;
				int peers = 0;
				int left = 0;

				T("action", action);
				T("transaction_id", r32(m));
				D("interval", r32(m));

				leechers = r32(m);
				seeders = r32(m);

				D("leechers", leechers);
				D("seeders", seeders);

				left = (len - 4 * 5);
				peers = left / (4 + 2);

				if (peers)
					printf("(%d peers)\n", peers);

				for (i = 0; i < peers; i++)
				{
					int addr = r32(m);
					unsigned char *ip = (unsigned char *)&addr;
					int port = r16(m);
					printf("%d.%d.%d.%d:%d\n", ip[3], ip[2], ip[1], ip[0], port);
				}

				if (peers)
					printf("(%d peers)\n", peers);
			}
			break;
		}

		case bt_scrape_request:
			T("connection_id", bt_connection_id = r64(m));
			T("action", r32(m));
			T("transaction_id", r32(m));
			break;

		case bt_scrape_response:
		{
			T("action", r32(m));
			T("transaction_id", r32(m));
			T("connection_id", bt_connection_id = r64(m));

			D("complete", r32(m));
			D("downloaded", r32(m));
			D("incomplete", r32(m));

			break;
		}
	}

	printf("End of packet\n");

	return 0;
}

int bt_make_packet(char *buf, int type, int buf_size)
{
	PACKET *m = &udp_ring[0];
	memset(buf, 0, buf_size);
	packet_init(m, buf, buf_size);

	switch (type)
	{
		case bt_connection_request:
		{
			w64(m, 0x041727101980LL);	//magic number
			w32(m, 0);
			w32(m, rand());
			break;
		}

		case bt_announce_request:
		{
			w64(m, bt_connection_id);
			w32(m, 1);	//action (1 - announce)
			w32(m, rand()); //transaction id
			wBuf(m, bt_info_hash, 20);	//info_hash
			wBuf(m, bt_peer_id, 20);	//software id
			w64(m, 0LL); //downloaded 
			w64(m, 0LL); //left
			w64(m, 0LL); //uploaded
			w32(m, bt_peer_event);	//0 - none, 1 - completed, 2 - started, 3 - stopped
			w32(m, 0);	// ip address, 0 for auto
			w32(m, bt_peer_key);
			w32(m, -1);	//num_want (max clients)
			w16(m, bt_peer_port);

			break;
		}

		case bt_scrape_request:
			w64(m, bt_connection_id);
			w32(m, 2);
			w32(m, rand());
			wBuf(m, bt_info_hash, 20);
			break;
	}

	return m->len;
}

int bt_udp_init(BT * bt, char *announce, char *info_hash, int client_port, int event)
{
	int port = 80;
	char host[64 + 1];

	sscanf(announce, "udp://%[^:]:%d/announce", host, &port);

	atoh(bt_info_hash, info_hash, 20);

	bt_peer_port = client_port;
	bt_peer_key = rand();
	bt->packet_type = bt_packet_type;
	bt->state = 1;

	net_open_link(&bt->link, host, port);
}

int bt_udp_update(BT * bt)
{
	const int buf_size = 4096;
	char buf[buf_size];
	int len = 0;

	switch (bt->state)
	{
		case 1:	//send message
			len = bt_make_packet(buf, bt->packet_type, buf_size);
			bt_parse_packet(buf, bt->packet_type, len);
			net_send(&bt->link, buf, len);
			bt->send_time = time(0);
			bt->packet_type++;
			bt->state = 2;
			break;

		case 2:	//wait for reply
			if ((len = net_recv(&bt->link, buf, buf_size)) > 0)
			{
				bt_parse_packet(buf, bt->packet_type, len);
				bt->packet_type++;
				bt->state = (bt->packet_type > bt_announce_response) ? 3 : 1;
			}
			else if (time(0) - bt->send_time > 4)
			{
				printf("[timeout]\n");
				bt->state = 3;
			}

			break;
	}

	return (bt->state > 0 && bt->state < 3);
}

void menu()
{
    char buf[256];

	printf("+--------------------------------------------------------+\n");
	printf("| Tracker: %-45s |\n", bt_tracker);
	printf("| Peer address: 0:%-5d                                  |\n", bt_peer_port);
	printf("| Peer software: \"%-20s\"                  |\n", bt_peer_id);
	printf("| info_hash: %-40s    |\n", htoa(buf, bt_info_hash, 20));
	printf("| a) Announce (add yourself and show peers)              |\n");
	printf("| r) Randomize peer (generate new port and client_id)    |\n");
	printf("| t) Randomize torrent (generate new info_hash)          |\n");
	printf("| s) Scrape (get info)                                   |\n");
	printf("| x) Remove peer (announce as stopped)                   |\n");
	printf("| q) Quit                                                |\n");
	printf("+--------------------------------------------------------+\n");
}

#ifndef main

int main(int argc, char **argv)
{
	const int buf_size = 4096;
	char buf[buf_size];
	int timeout = 1000;
	int len = 0;
	char cmd = 0;

	srand(time(0));

	if (argc > 1 && argc < 3)
		printf("usage: [udp://tracker.name:80/announce] [info_hash (40 chars)] <action (r,t,a,x,s)>"), exit(0);

	if (argc > 1)
		bt_tracker = argv[1];

	if (argc > 2)
		bt_str_hash = argv[2];

	if (argc > 3)
		cmd = argv[3][0];

	net_init();

	atoh(bt_info_hash, bt_str_hash, 20);

	BT bt;

	while (1)
	{
		int i = 0;
		int ch = cmd;

		menu();
		printf("udp>");
		len = strlen(fgets(buf, buf_size, stdin));
		ch = buf[0];

		switch (ch)
		{
			case 'r':
				bt_peer_port = rand() % 32767;
				bt_peer_key = rand();
				rndstr(bt_peer_id + 8, '0', '9', 20 - 8);
				bt_packet_type = 0;
				break;

			case 't':
				rndstr(bt_info_hash, 0, 0xff, 20);
				break;

			case 'a':
				bt_peer_event = 2;	//started
				bt_packet_type = bt_connection_request;
				bt_udp_init(&bt, bt_tracker, bt_info_hash, 666, 0);
				break;

			case 'x':
				bt_peer_event = 3;	//stopped
				bt_packet_type = bt_connection_request;
				bt_udp_init(&bt, bt_tracker, bt_info_hash, 666, 0);
				break;

			case 's':
				bt_packet_type = bt_scrape_request;
				bt_udp_init(&bt, bt_tracker, bt_info_hash, 666, 0);
				break;

			case 'q':
				exit(0);
				break;

			default:
				bt_packet_type = 0;
		}

		while (bt_udp_update(&bt))
		{
			printf(".");
			Sleep(40);
		}
	}

	net_close();

	return 0;
}

#endif
