/*
	UDP Game Client
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Network.h"
#include "TURN.h"
#include "STUN.h"

using namespace std;
using namespace net;

int main(int argc, char **argv)
{
	const int ServerPort = 30000;
	const int ClientPort = 30001;

	if ( !InitializeSockets() )
	{
		printf( "failed to initialize sockets\n" );
		return 1;
	}
	
	STUN stun;

	Address peer = stun.GetAddress();
	
	ShutdownSockets();
	
	return 0;
}
