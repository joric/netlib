#ifndef STUN_H
#define STUN_H

#include "Network.h"
#include "Packet.h"
#include "Utils.h"

namespace net
{	
	class STUN
	{
	public:	

		int WriteHeader(Packet* p, int type)
		{
			char tsx[12];
			RandomKey(tsx, 0, 0xff, 12);
			p->w16(type);
			p->w16(0);
			p->w32(MagicCookie);
			p->wBuf(tsx, 12);
			return 0;
		}

		int WriteFooter(Packet* p)
		{
			p->ofs = 2;
			p->w16(p->len - HeaderSize);
			return 0;
		}

		Address GetAddress(char* host, int port)
		{
			ServerHost = "stun.xten.com";
			//ServerHost = "numb.viagenie.ca";
			ServerPort = 3478;
			ClientPort = 30001;

			const float DeltaTime = 0.25f;
			const float TimeOut = 2.0f;

			HeaderSize = 20;
			MagicCookie = 0x2112A442;

			Connection connection( 0, TimeOut );

			connection.Start( ClientPort );		

			connection.Connect( Address(ServerHost, ServerPort) );


			unsigned char data[1024];

			Packet packet(data, 1024);

			WriteHeader(&packet, MethodBinding);
			WriteFooter(&packet);

			bool connected = false;
			int bytes_read = 0;

			while ( true )
			{
				if ( !connected && connection.IsConnected() )
				{
					printf( "client connected to server\n" );
					connected = true;
				}

				if ( !connected && connection.ConnectFailed() )
				{
					printf( "connection failed\n" );
					break;
				}

				connection.SendPacket( packet.buf, packet.len );

				while ( true )
				{
					unsigned char packet[256];
					bytes_read = connection.ReceivePacket( packet, sizeof(packet) );
					if ( bytes_read == 0 )
						break;

					DumpHex(packet, bytes_read);

					printf( "received packet from server (%d bytes) \n", bytes_read );
				}

				connection.Update( DeltaTime );
				wait( DeltaTime );

				if (bytes_read != 0)
					break;
			}

			return Address();
		};


		enum {
			MethodBinding = 0x0001,
			MethodSharedSecret = 0x0002,
			MethodAllocate = 0x0003,
			MethodRefresh = 0x0004,
			MethodSend = 0x0006,
			MethodData = 0x0007,
			MethodCreatePermission = 0x0008,
			MethodChannelBind = 0x0009,
		};

		char* ServerHost;		
		int ServerPort;
		int ClientPort;
		int MagicCookie;
		int HeaderSize;
	};
}

#endif 


