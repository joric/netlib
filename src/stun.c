//public domain

#ifndef __STUN_H__
#define __STUN_H__

#define main

#include "utils.c"
#include "link.c"
#include "packet.c"

#undef main

typedef struct {
	ADDRESS base_address;
	ADDRESS mapped_address;
	ADDRESS changed_address;
	LINK link;
	int received;
	int sent;
	int state;
	int msg_code;
	int finished;
	int send_time;
	int timeout;
	int res;
} STUN;

enum {
	STUN_HEADER_SIZE = 20,
	STUN_MAGIC_COOKIE = 0x2112A442,
	STUN_BINDING_METHOD = 1,
	STUN_SHARED_SECRET_METHOD = 2,
	STUN_ALLOCATE_METHOD = 3,
	STUN_REFRESH_METHOD = 4,
	STUN_SEND_METHOD = 6,
	STUN_DATA_METHOD = 7,
	STUN_CHANNEL_BIND_METHOD = 9,
	STUN_REQUEST_CLASS = 0,
	STUN_INDICATION_CLASS = 1,
	STUN_SUCCESS_CLASS = 2,
	STUN_ERROR_CLASS = 3,
	STUN_PRESERVES_PORT = 64,
	STUN_NAT = 128,
};

enum stun_vars {
	STUN_MAPPED_ADDRESS = 0x0001,
	STUN_RESPONSE_ADDRESS = 0x0002,
	STUN_CHANGE_REQUEST = 0x0003,
	STUN_SOURCE_ADDRESS = 0x0004,
	STUN_CHANGED_ADDRESS = 0x0005,
	STUN_USERNAME = 0x0006,
	STUN_PASSWORD = 0x0007,
	STUN_MESSAGE_INTEGRITY = 0x0008,
	STUN_ERROR_CODE = 0x0009,
	STUN_UNKNOWN_ATTRIBUTES = 0x000A,
	STUN_REFLECTED_FROM = 0x000B,
	STUN_CHANNEL_NUMBER = 0x000C,
	STUN_LIFETIME = 0x000D,
	STUN_BANDWIDTH = 0x0010,
	STUN_PEER_ADDRESS = 0x0012,
	STUN_DATA = 0x0013,
	STUN_REALM = 0x0014,
	STUN_NONCE = 0x0015,
	STUN_RELAYED_ADDRESS = 0x0016,
	STUN_REQUESTED_ADDRESS_TYPE = 0x0017,
	STUN_REQUESTED_PROPS = 0x0018,
	STUN_REQUESTED_TRANSPORT = 0x0019,
	STUN_XOR_MAPPED_ADDRESS = 0x8020,
	STUN_TIMER_VAL = 0x0021,
	STUN_RESERVATION_TOKEN = 0x0022,
	STUN_XOR_REFLECTED_FROM = 0x0023,
	STUN_PRIORITY = 0x0024,
	STUN_USE_CANDIDATE = 0x0025,
	STUN_ICMP = 0x0030,
	STUN_END_MANDATORY_ATTR,
	STUN_START_EXTENDED_ATTR = 0x8021,
	STUN_SOFTWARE = 0x8022,
	STUN_ALTERNATE_SERVER = 0x8023,
	STUN_REFRESH_INTERVAL = 0x8024,
	STUN_FINGERPRINT = 0x8028,
	STUN_ICE_CONTROLLED = 0x8029,
	STUN_ICE_CONTROLLING = 0x802A,
};

#define TN(t) printf("0x%04d", t)
#define T(id) if (type == id) return #id+5; else

char *STUN_METHOD_NAME(int type)
{
    T(STUN_BINDING_METHOD);
	T(STUN_SHARED_SECRET_METHOD);
	T(STUN_ALLOCATE_METHOD);
	T(STUN_REFRESH_METHOD);
	T(STUN_SEND_METHOD);
	T(STUN_DATA_METHOD);
	T(STUN_CHANNEL_BIND_METHOD);
    TN(type);
}

char *STUN_ATTR_NAME(int type)
{
	T(STUN_MAPPED_ADDRESS);
	T(STUN_RESPONSE_ADDRESS);
	T(STUN_CHANGE_REQUEST);
	T(STUN_SOURCE_ADDRESS);
	T(STUN_CHANGED_ADDRESS);
	T(STUN_USERNAME);
	T(STUN_PASSWORD);
	T(STUN_MESSAGE_INTEGRITY);
	T(STUN_ERROR_CODE);
	T(STUN_UNKNOWN_ATTRIBUTES);
	T(STUN_REFLECTED_FROM);
	T(STUN_CHANNEL_NUMBER);
	T(STUN_LIFETIME);
	T(STUN_BANDWIDTH);
	T(STUN_PEER_ADDRESS);
	T(STUN_DATA);
	T(STUN_REALM);
	T(STUN_NONCE);
	T(STUN_RELAYED_ADDRESS);
	T(STUN_REQUESTED_ADDRESS_TYPE);
	T(STUN_REQUESTED_PROPS);
	T(STUN_REQUESTED_TRANSPORT);
	T(STUN_XOR_MAPPED_ADDRESS);
	T(STUN_TIMER_VAL);
	T(STUN_RESERVATION_TOKEN);
	T(STUN_XOR_REFLECTED_FROM);
	T(STUN_PRIORITY);
	T(STUN_USE_CANDIDATE);
	T(STUN_ICMP);
	T(STUN_END_MANDATORY_ATTR);
	T(STUN_START_EXTENDED_ATTR);
	T(STUN_SOFTWARE);
	T(STUN_ALTERNATE_SERVER);
	T(STUN_REFRESH_INTERVAL);
	T(STUN_FINGERPRINT);
	T(STUN_ICE_CONTROLLED);
	T(STUN_ICE_CONTROLLING);
	TN(type);
};

void stun_write_header(PACKET * m, int type)
{
	char tsx_id[12];
	rndstr(tsx_id, 0, 0xff, 12);
	w16(m, type);
	w16(m, 0);
	w32(m, STUN_MAGIC_COOKIE);
	wBuf(m, tsx_id, 12);
}

void stun_write_footer(PACKET * m)
{
	m->ofs = 2;
	w16(m, m->len - STUN_HEADER_SIZE);
}

int stun_xor_address(ADDRESS * addr)
{
	int i;
	int x = htonl(STUN_MAGIC_COOKIE);
	char *p = (char *)&x;
	int msb = (((char *)&x)[0] << 8) | ((char *)&x)[1];
	addr->sin_port ^= htons(msb);
	char *ip = (char *)&addr->sin_addr.s_addr;
	for (i = 0; i < 4; i++)
		ip[i] ^= p[i];
}

int stun_parse_address(PACKET * m, ADDRESS * addr)
{
	addr->sin_family = (r16(m) == 1) ? 2 : 1;
	addr->sin_port = htons(r16(m));
	char *p = (char *)&addr->sin_addr.s_addr;
	rBuf(m, p, 4);
}

int stun_parse(STUN * stun, PACKET * m)
{
	m->ofs = 0;
	int type = r16(m);
	int length = r16(m);
	int magic = r32(m);
	char tsx_id[12];
    char buf[1024];

	if (magic != STUN_MAGIC_COOKIE)
		return 0;

	rBuf(m, tsx_id, 12);

	int msg = type & ~0x110;
	int code = type & 0x110;

	printf(" Message: %s (%d)\n", STUN_METHOD_NAME(msg), code);
	printf("  hdr: length=%d, magic=0x%x, tsx_id=%s", length, magic, htoa(buf, tsx_id, 12));
	printf("\n");
	printf("  Attributes:\n");

	int offset = m->ofs;

	while ((offset - STUN_HEADER_SIZE) < length)
	{
		int attr = r16(m);
		int len = r16(m);
		printf("  %s length=%d, ", STUN_ATTR_NAME(attr), len);
		switch (attr)
		{
			case STUN_MAPPED_ADDRESS:
			case STUN_RESPONSE_ADDRESS:
			case STUN_SOURCE_ADDRESS:
			case STUN_CHANGED_ADDRESS:
			case STUN_XOR_MAPPED_ADDRESS:
			{
				ADDRESS addr;

				stun_parse_address(m, &addr);

				if (attr == STUN_XOR_MAPPED_ADDRESS)
					stun_xor_address(&addr);

				printf(net_to_string(&addr));

				if (attr == STUN_MAPPED_ADDRESS)
					memcpy(&stun->mapped_address, &addr, sizeof(ADDRESS));

				if (attr == STUN_CHANGED_ADDRESS)
					memcpy(&stun->changed_address, &addr, sizeof(ADDRESS));

				break;
			}

			case STUN_SOFTWARE:
				printf(m->buf + m->ofs);
				break;

			default:
				printf("%s", htoa(buf, m->buf + m->ofs, len));
				break;
		}

		printf("\n");
		len = pad(len, 4);
		offset += len + 4;
		m->ofs = offset;
	}

	return 1;
}

void stun_write_attr(PACKET * m, int attr, char *buf, int len)
{
	int dt = pad(len, 4) - len;
	w16(m, attr);
	w16(m, len);
	wBuf(m, buf, len);
	m->ofs += dt;
}

#define stun_write_str(m,attr,str) stun_write_attr(m, attr, str, strlen(str));
#define stun_write_uint(m,attr,value) w16(m, attr); w16(m, 4); w32(m, value);

int stun_recv(STUN * stun, char *buf, int buf_size)
{
	ADDRESS addr;
	int addr_size = sizeof(ADDRESS);

	int len = recvfrom(stun->link.sock, buf, buf_size, 0, (struct sockaddr *)&addr, &addr_size);

	if (len > 0)
	{
		printf("[recv %d bytes from %s]\n", len, net_to_string(&addr));
		PACKET m;
		packet_init(&m, buf, buf_size);
		if (stun_parse(stun, &m))
		{
			stun->received++;
			return len;
		}
	}

	return 0;
}

int stun_send_message(STUN * stun, int type)
{
	LINK *link = &stun->link;
	PACKET mp;
	PACKET *m = &mp;

	char buf[1024];
	int buf_size = 1024;

	packet_init(m, buf, buf_size);

//    printf("base: %s\n", net_to_string(&stun->base_address));

	net_set_address(link, &stun->base_address);

	switch (type)
	{
		case 0:	//binding
			stun_write_header(m, STUN_BINDING_METHOD);
			break;

		case 1:	// Test I 
			stun_write_header(m, STUN_BINDING_METHOD);
			break;

		case 2:	// Test II
			stun_write_header(m, STUN_BINDING_METHOD);
			stun_write_uint(m, STUN_CHANGE_REQUEST, 4 + 2);	//change addr & port
			break;

		case 3:	// Test I(2)
			net_set_address(link, &stun->changed_address);
			stun_write_header(m, STUN_BINDING_METHOD);
			break;

		case 4:	// Test III
			stun_write_header(m, STUN_BINDING_METHOD);
			stun_write_uint(m, STUN_CHANGE_REQUEST, 2);	//change port
			break;

		case 5:	//refresh
			stun_write_header(m, STUN_REFRESH_METHOD);
			break;

		default:
			stun_write_header(m, STUN_BINDING_METHOD);
			break;
	}

	stun_write_footer(m);

	if (m->len)
	{
        int i;
        for (i = 0; i<m->len; i++)
            printf("\\x%02x", (unsigned char)m->buf[i]);
        printf("\n");

		stun->send_time = time(0);
		stun->sent++;
		net_send(link, m->buf, m->len);


		stun_parse(stun, m);
	}

	return m->len;
}

int stun_init(STUN * stun, char *host, int port, int action)
{
	net_open_link(&stun->link, host, port);
	stun->timeout = 4;
	stun->res = 0;
	stun->state = 1;
	stun->msg_code = action;
	stun->finished = 0;
	stun->sent = 0;
	stun->received = 0;
	memcpy(&stun->base_address, &stun->link.addr, sizeof(ADDRESS));
}

int stun_update(STUN * stun)
{
//    printf("STATE: %d\n", stun->state);   
//    printf("link state: %d\n", stun->link.state);   

	ADDRESS addr;
	int addr_size = sizeof(ADDRESS);
	int buf_size = 8192;
	char buf[buf_size];
	int len = 0;

	switch (stun->state)
	{
		case 1:	//send message
			printf("sending message %d\n", stun->msg_code);
			stun_send_message(stun, stun->msg_code);
			stun->send_time = time(0);
			stun->state = 2;
			break;

		case 2:	//receive message
			if (stun_recv(stun, buf, buf_size) > 0)
				stun->state = 4;
			else	//timeout
			if (time(0) - stun->send_time > stun->timeout)
				stun->state = 3;
			break;

		case 3:	//timeout
			printf("[timeout]\n");
			stun->timeout = 5;

			if (stun->msg_code == 0 || stun->msg_code == 4)
				stun->finished = 1;

			if (stun->msg_code == 2 && !(stun->res & STUN_NAT))
				stun->finished = 1;
			stun->state = stun->finished ? 5 : 1;
			stun->msg_code++;

			break;
		case 4:	//received
			printf("received reply to message %d\n", stun->msg_code);

			stun->timeout = time(0) - stun->send_time + 2;

			if (stun->msg_code == 1 && !net_is_local_address(&stun->mapped_address))
				stun->res |= STUN_NAT;

			if (stun->msg_code != 1 && stun->mapped_address.sin_port == stun->link.port)
				stun->res |= STUN_PRESERVES_PORT;

			stun->res |= 1 << (stun->msg_code - 1);

			if (stun->msg_code == 0 || stun->msg_code == 4)
				stun->finished = 1;

			stun->state = stun->finished ? 5 : 1;
			stun->msg_code++;

			break;

		case 5:	//wait keep-alive seconds
			if (time(0) - stun->send_time > 10)
    			stun->state = 6;
    		if (stun_recv(stun, buf, buf_size) > 0)
				stun->state = 6;
			break;

		case 6:	//send keep-alive
			printf("send keep-alive\n");
			stun_send_message(stun, 5);
			stun->state = 5;
			break;
	}
	return !stun->finished;
}

#endif //__STUN_H__

#include <stdio.h>

#define BIN__N(x) (x) | x>>3 | x>>6 | x>>9
#define BIN__B(x) (x) & 0xf | (x)>>12 & 0xf0
#define BIN(v) (BIN__B(BIN__N(0x##v)))

char *itob(int x)
{
	static char buf[8 + 1] = { 0 };
	int i;
	for (i = 0; i < 8; i++)
		buf[8 - 1 - i] = (x & (1 << i)) ? '1' : '0';
	return buf;
}

int main(int argc, char **argv)
{
	STUN stun;

	int port = 3478;
	char *host = "stun.xten.com";
//char *host = "stun.ideasip.com";

	if (argc > 1)
		host = argv[1];

	net_init();

	stun_init(&stun, host, port, 1);	//1 - check connection, 0 - binding + keepalives

	while (stun_update(&stun))
	{
		printf(".");
		Sleep(40);
	}

	char *type = "Restricted NAT";

	switch (stun.res & BIN(1111))
	{
		case BIN(0000):
			type = "UDP Blocked";
			break;
		case BIN(0001):
			type = "UDP Firewall";
			break;
		case BIN(0011):
			type = "Open Internet";
			break;
		case BIN(1101):
			type = "Full Cone NAT";
			break;
	}

	printf("res: %s\n", itob(stun.res));
	printf("NAT present: %s\n", (stun.res & STUN_NAT) ? "yes" : "no");
	printf("preserves port: %s\n", (stun.res & STUN_PRESERVES_PORT) ? "yes" : "no");
	printf("type: %s\n", type);


	   //keep sending keep-alives
	   while (1)
	   {
	   printf(".");
	   stun_update(&stun);
	   Sleep(40);
	   }

}
