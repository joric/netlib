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
#define SOCKET int
#define closesocket close
#define Sleep(t) usleep(t*1000)
#endif
#define ADDRESS struct sockaddr_in

typedef struct {
	ADDRESS address;
	SOCKET socket;
	SOCKET listener;
	char *host;
	int send_time;
	int recv_time;
	int index;
	int state;
	int server;
} icmp_t;

icmp_t m_icmp;

struct ip_packet_t {
	unsigned char vers_ihl, tos;
	unsigned short pkt_len, id, flags_frag_offset;
	unsigned char ttl, proto;	// 1 for ICMP
	unsigned short checksum;
	unsigned int src_ip, dst_ip;
};


/*	icmp_packet_t: This is the definition of a standard ICMP header. */
struct icmp_packet_t {
	unsigned char type, code;
	unsigned short checksum, identifier, seq;
};

#ifndef IP_MAX_SIZE
#define IP_MAX_SIZE 65535
#endif

#ifndef IPHDR_SIZE
#define IPHDR_SIZE  sizeof(struct ip_packet_t)
#endif
#ifndef ICMPHDR_SIZE
#define ICMPHDR_SIZE  sizeof(struct icmp_packet_t)
#endif

#define IPDEFTTL 64

int send_icmp(int icmp_sock, struct sockaddr_in *rsrc, struct sockaddr_in *dest_addr, struct sockaddr_in *src_addr, int server);
unsigned short calc_icmp_checksum(unsigned short *data, int bytes);

void socket_broadcast(int sd)
{
	const int one = 1;

	if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (char *)&one, sizeof(one)) == -1)
	{
		printf("[socket_broadcast] can't set SO_BROADCAST option\n");
		/* non fatal error */
	}
}

void socket_nonblock(int sd)
{
	unsigned long nonblock = 1;
	int res = 0;

#ifdef _WIN32
	res = ioctlsocket(sd, FIONBIO, &nonblock);
#else
	res = fcntl(sd, F_SETFL, O_NONBLOCK, nonblock);
#endif

	if (res == -1)
	{
		printf("[socket_nonblock] can't set NONBLOCK option\n");
		/* non fatal error */
	}
}

void socket_iphdrincl(int sd)
{
	const int one = 1;

	if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, (char *)&one, sizeof(one)) == -1)
	{
		printf("[socket_iphdrincl] can't set IP_HDRINCL option\n");
		/* non fatal error */
	}
}

int create_listen_socket()
{
	int listen_sock;

	listen_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	if (listen_sock < 0)
	{
		printf("Couldn't create privileged icmp socket: %s\n", strerror(errno));
		return 0;
	}

#ifdef WIN32
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

	struct hostent *h;
	struct sockaddr_in sa;
	unsigned int opt = 1;
	DWORD bytes;

#define HOSTNAME_LEN 64

	char hostname[HOSTNAME_LEN];

	if ((gethostname(hostname, HOSTNAME_LEN)) == -1)
		printf("unable to gethostname\n");

	if ((h = gethostbyname(hostname)) == NULL)
		printf("unable to gethostbyname\n");

//    h = gethostbyname("127.0.0.1");

	sa.sin_family = AF_INET;
	sa.sin_port = htons(0);
	memcpy(&sa.sin_addr.s_addr, h->h_addr_list[0], h->h_length);

//      printf("hostname: %s\n", inet_ntoa(sa.sin_addr));

      if ((bind(listen_sock, (struct sockaddr *) & sa, sizeof(sa))) == -1)
              printf("unable to bind() socket\n");


	if ((WSAIoctl(listen_sock, SIO_RCVALL, &opt, sizeof(opt), NULL, 0, &bytes, NULL, NULL)) == SOCKET_ERROR)
		printf("failed to set promiscuous mode\n");
#endif

	socket_nonblock(listen_sock);

	return listen_sock;
}

int create_icmp_socket()
{
	int icmp_sock;

	icmp_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (icmp_sock < 0)
	{
		printf("Couldn't create privileged raw socket: %s\n", strerror(errno));
		return 0;
	}

//	socket_broadcast(icmp_sock);
//  socket_iphdrincl(icmp_sock);

	return icmp_sock;
}


/* Send an ICMP time exceeded packet */
int send_icmp(int icmp_sock, struct sockaddr_in *rsrc, struct sockaddr_in *dest_addr, struct sockaddr_in *src_addr, int server)
{
	int pkt_len = IPHDR_SIZE + ICMPHDR_SIZE, err = 0;
	struct ip_packet_t *ip_pkt;
	struct ip_packet_t *ip_pkt2;
	struct icmp_packet_t *pkt;
	struct icmp_packet_t *pkt2;
	char *packet;

	if (!server)
		pkt_len *= 2;

	packet = (char *)malloc(pkt_len);
	memset(packet, 0, pkt_len);

	ip_pkt = (struct ip_packet_t *)packet;
	ip_pkt->vers_ihl = 0x45;	//|(pkt_len>>2);//5;//(IPVERSION << 4) | (IPHDR_SIZE >> 2);
	ip_pkt->tos = 0;
	ip_pkt->pkt_len = pkt_len;
	ip_pkt->id = 1;		//kernel sets proper value htons(ip_id_counter);
	ip_pkt->flags_frag_offset = 0;
	ip_pkt->ttl = IPDEFTTL;	// default time to live (64)
	ip_pkt->proto = 1;	// ICMP
	ip_pkt->checksum = 0;	// maybe the kernel helps us out..?
	ip_pkt->src_ip = rsrc->sin_addr.s_addr;	// insert source IP address here
	ip_pkt->dst_ip = dest_addr->sin_addr.s_addr;

	pkt = (struct icmp_packet_t *)malloc(ICMPHDR_SIZE);
	memset(pkt, 0, ICMPHDR_SIZE);
	pkt->type = server ? 8 : 11;	// ICMP echo request or time exceeded
	pkt->code = 0;		// Must be zero 
	pkt->identifier = 0;
	pkt->seq = 0;
	pkt->checksum = 0;

	/* Generate "original" packet if client to append to time exceeded */
	if (!server)
	{
		ip_pkt2 = (struct ip_packet_t *)malloc(IPHDR_SIZE);
		memset(ip_pkt2, 0, IPHDR_SIZE);
		ip_pkt2->vers_ihl = 0x45;
		ip_pkt2->tos = 0;
		/* no idea why i need to shift the bits here, but not on ip_pkt->pkt_len... */
		ip_pkt2->pkt_len = (IPHDR_SIZE + ICMPHDR_SIZE) << 8;
		ip_pkt2->id = 1;	//kernel sets proper value htons(ip_id_counter);
		ip_pkt2->flags_frag_offset = 0;
		ip_pkt2->ttl = 1;	// real TTL would be 1 on a time exceeded packet
		ip_pkt2->proto = 1;	// ICMP
		ip_pkt2->checksum = 0;	// maybe the kernel helps us out..?
		ip_pkt2->src_ip = dest_addr->sin_addr.s_addr;	//htonl(0x7f000001); // localhost..
		ip_pkt2->dst_ip = src_addr->sin_addr.s_addr;	//htonl(0x7f000001); // localhost..

		pkt2 = (struct icmp_packet_t *)malloc(ICMPHDR_SIZE);
		memset(pkt2, 0, ICMPHDR_SIZE);
		pkt2->type = 8;	// ICMP echo request
		pkt2->code = 0;	// Must be zero 
		pkt2->identifier = 0;
		pkt2->seq = 0;
		pkt2->checksum = 0;

		pkt2->checksum = htons(calc_icmp_checksum((unsigned short *)pkt2, ICMPHDR_SIZE));
		ip_pkt2->checksum = htons(calc_icmp_checksum((unsigned short *)ip_pkt2, IPHDR_SIZE));
	}

	pkt->checksum = htons(calc_icmp_checksum((unsigned short *)pkt, ICMPHDR_SIZE));
	ip_pkt->checksum = htons(calc_icmp_checksum((unsigned short *)ip_pkt, IPHDR_SIZE));

	memcpy(packet + IPHDR_SIZE, pkt, ICMPHDR_SIZE);

	if (!server)
	{
		memcpy(packet + IPHDR_SIZE + ICMPHDR_SIZE, ip_pkt2, IPHDR_SIZE);
		memcpy(packet + IPHDR_SIZE + ICMPHDR_SIZE + IPHDR_SIZE, pkt2, ICMPHDR_SIZE);
	}

	printf("ICMP: sending (%d bytes) to %s  from socket %d\n", pkt_len, inet_ntoa(dest_addr->sin_addr), icmp_sock);

	err = sendto(icmp_sock, packet, pkt_len, 0, (struct sockaddr *)dest_addr, sizeof(struct sockaddr));

	free(packet);

	//err = sendto(icmp_sock, (const void*)ip_pkt, pkt_len, 0, (struct sockaddr*)dest_addr, sizeof(struct sockaddr));

	if (err < 0)
	{
		printf("Failed to send ICMP packet: %s\n", strerror(errno));


		return -1;
	}
	else if (err != pkt_len)
		printf("WARNING WARNING, didn't send entire packet\n");

	return 0;
}

void icmp_parse(char* buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        unsigned char c = buf[i];
        
        printf("%c", c >=32 && c <= 128 ? c : '.');
    }

    printf("\n\n");
}

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


int icmp_init(icmp_t * icmp)
{

	printf("initializing %s at %s\n", icmp->server ? "server" : "client", icmp->host);

	icmp->socket = create_icmp_socket();

	if (icmp->server)
		icmp->listener = create_listen_socket();

	icmp->send_time = time(0);
	icmp->state = 1;
	return 0;
}

int icmp_recv(icmp_t * icmp)
{
	int ip;
	char ips[16];
	char packet[IP_MAX_SIZE];
	struct hostent *host_ent, *hp;
	unsigned int timeexc_ip;

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(struct sockaddr_in));

	sa.sin_family = PF_INET;
	sa.sin_port = htons(2222);
	sa.sin_addr.s_addr = INADDR_ANY;

	/* Wait for random client to penetrate our NAT...you nasty client! */
	while ((ip = recv(icmp->listener, packet, IP_MAX_SIZE, 0)) > 0)
	{
		printf("ICMP: got something (%d bytes)\n", ip);

        icmp_parse(packet, ip);

		/* If not ICMP and not TTL exceeded */
		if (packet[9] != 1 || packet[20] != 11 || packet[21] != 0)
			break;

		sprintf(ips, "%d.%d.%d.%d", packet[12], packet[13], packet[14], packet[15]);
		memset(packet, 0, ip);

		host_ent = gethostbyname(ips);
		memcpy(&sa.sin_addr, host_ent->h_addr, host_ent->h_length);

		//inet_pton(PF_INET, ips, &(sa.sin_addr));

		/* Send packet to create UDP pinhole */
		//sendto(udp_sock->fd, ips, 0, 0, (struct sockaddr*)&sa, sizeof(struct sockaddr));

		printf("PINHOLE %s:%d!\n", inet_ntoa(sa.sin_addr), sa.sin_port);
	}

	return 0;
}

int icmp_update(icmp_t * icmp)
{
	if (icmp->state == 0)
		return 0;

	struct sockaddr_in dest, rsrc, src;
	struct hostent *host_ent, *hp;
	unsigned int timeexc_ip;

	memset(&dest, 0, sizeof(struct sockaddr_in));
	host_ent = gethostbyname("3.3.3.3");
	//host_ent = gethostbyname("127.0.0.1");
	timeexc_ip = *(unsigned int *)host_ent->h_addr_list[0];
	dest.sin_family = AF_INET;
	dest.sin_port = 0;
	dest.sin_addr.s_addr = timeexc_ip;

	memset(&src, 0, sizeof(struct sockaddr_in));
	hp = gethostbyname(icmp->host);
	timeexc_ip = *(unsigned int *)hp->h_addr_list[0];
	src.sin_family = AF_INET;
	src.sin_port = 0;
	src.sin_addr.s_addr = timeexc_ip;

	if (icmp->server)
		icmp_recv(icmp);

	//refresh
	if (icmp->send_time + 2 <= time(0))
	{
		icmp->send_time = time(0);

		if (icmp->server)
		{
			send_icmp(icmp->socket, &rsrc, &dest, (struct sockaddr_in *)0, 1);
		}
		else
		{
			rsrc.sin_family = PF_INET;
			rsrc.sin_addr.s_addr = INADDR_ANY;
			send_icmp(icmp->socket, &rsrc, &src, &dest, 0);
		}
	}

	return 0;
}

int main(int argc, char **argv)
{

	icmp_t m_icmp;
	icmp_t m_icmpc;

#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	m_icmp.server = 1;
	m_icmp.host = "127.0.0.1";

	m_icmpc.server = 0;
	m_icmpc.host = "127.0.0.1";


	int server = 1;
	int client = 0;

	if (argc > 1)
	{
		server = 0;
		client = 1;

		m_icmpc.host = argv[1];
	}


	if (server)
		icmp_init(&m_icmp);

	if (client)
		icmp_init(&m_icmpc);

	while (1)
	{
		if (server)
			icmp_update(&m_icmp);

		if (client)
			icmp_update(&m_icmpc);

		Sleep(1);
	}

	return 0;
}
