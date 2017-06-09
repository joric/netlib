// connection

#include "Network.h"

class Connection
{
public:

	enum Mode
	{
		None,	
		Client,
		Server
	};

	Connection( unsigned int protocolId, float timeout )
	{
		this->protocolId = protocolId;
		this->timeout = timeout;
		mode = None;
		running = false;
		ClearData();
	}

	~Connection()
	{
		if ( running )
			Stop();
	}

	bool Start( int port )
	{
		assert( !running );
		printf( "start connection on port %d\n", port );
		if ( !socket.Open( port ) )
			return false;
		running = true;
		return true;
	}

	void Stop()
	{
		assert( running );
		printf( "stop connection\n" );
		ClearData();
		socket.Close();
		running = false;
	}

	void Listen()
	{
		printf( "server listening for connection\n" );
		ClearData();
		mode = Server;
		state = Listening;
	}

	void Connect( const Address & address )
	{		
		printf( "client connecting to %d.%d.%d.%d:%d\n", 
			address.GetA(), address.GetB(), address.GetC(), address.GetD(), address.GetPort() );
		ClearData();
		mode = Client;
		state = Connecting;
		this->address = address;
	}

	bool IsConnecting() const
	{
		return state == Connecting;
	}

	bool ConnectFailed() const
	{
		return state == ConnectFail;
	}

	bool IsConnected() const
	{
		return state == Connected;
	}

	bool IsListening() const
	{
		return state == Listening;
	}

	Mode GetMode() const
	{
		return mode;
	}

	void Update( float deltaTime )
	{
		assert( running );
		timeoutAccumulator += deltaTime;
		if ( timeoutAccumulator > timeout )
		{
			if ( state == Connecting )
			{
				printf( "connect timed out\n" );
				ClearData();
				state = ConnectFail;
			}
			else if ( state == Connected )
			{
				printf( "connection timed out\n" );
				ClearData();
				if ( state == Connecting )
					state = ConnectFail;
			}
		}
	}

	bool SendPacket( const unsigned char data[], int size )
	{
		assert( running );
		if ( address.GetAddress() == 0 )
			return false;
		return socket.Send( address, data, size );
	}

	int ReceivePacket( unsigned char data[], int size )
	{
		assert( running );
		Address sender;
		int bytes_read = socket.Receive( sender, data, size );
		if ( bytes_read == 0 )
			return 0;			
		if ( mode == Server && !IsConnected() )
		{
			printf( "server accepts connection from client %d.%d.%d.%d:%d\n", 
				sender.GetA(), sender.GetB(), sender.GetC(), sender.GetD(), sender.GetPort() );
			state = Connected;
			address = sender;
		}
		if ( sender == address )
		{
			if ( mode == Client && state == Connecting )
			{
				printf( "client completes connection with server\n" );
				state = Connected;
			}
			timeoutAccumulator = 0.0f;
			return size;
		}
		return 0;
	}

protected:

	void ClearData()
	{
		state = Disconnected;
		timeoutAccumulator = 0.0f;
		address = Address();
	}

private:

	enum State
	{
		Disconnected,
		Listening,
		Connecting,
		ConnectFail,
		Connected
	};

	unsigned int protocolId;
	float timeout;

	bool running;
	Mode mode;
	State state;
	Socket socket;
	float timeoutAccumulator;
	Address address;
};
