// internet address

#include "Network.h"

class Address
{
public:

	Address()
	{
		address = 0;
		port = 0;
	}

	Address( char* address, unsigned short port )
	{
		struct hostent *addr = gethostbyname(address);

		this->address = htonl(*(unsigned int*)addr->h_addr_list[0]);
		this->port = port;
	}

	Address( unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port )
	{
		this->address = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
		this->port = port;
	}

	Address( unsigned int address, unsigned short port )
	{
		this->address = address;
		this->port = port;
	}

	unsigned int GetAddress() const
	{
		return address;
	}

	unsigned char GetA() const
	{
		return ( unsigned char ) ( address >> 24 );
	}

	unsigned char GetB() const
	{
		return ( unsigned char ) ( address >> 16 );
	}

	unsigned char GetC() const
	{
		return ( unsigned char ) ( address >> 8 );
	}

	unsigned char GetD() const
	{
		return ( unsigned char ) ( address );
	}

	unsigned short GetPort() const
	{ 
		return port;
	}

	bool operator == ( const Address & other ) const
	{
		return address == other.address && port == other.port;
	}

	bool operator != ( const Address & other ) const
	{
		return ! ( *this == other );
	}

	bool operator < ( const Address & other ) const
	{
		// note: this is so we can use address as a key in std::map
		if ( address < other.address )
			return true;
		if ( address > other.address )
			return false;
		else
			return port < other.port;
	}

private:
	unsigned int address;
	unsigned short port;
};
