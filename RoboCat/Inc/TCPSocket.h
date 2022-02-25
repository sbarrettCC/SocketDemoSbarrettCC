class TCPSocket
{
public:
	~TCPSocket();	//I tried making this private after implementing CleanupSocket(), but it yelled at me. Keeping this here for now.
	int								Connect( const SocketAddress& inAddress );
	int								Bind( const SocketAddress& inToAddress );
	int								Listen( int inBackLog = 32 );
	shared_ptr< TCPSocket >			Accept( SocketAddress& inFromAddress );
	int32_t							Send( const void* inData, size_t inLen );
	int32_t							Receive( void* inBuffer, size_t inLen );
	int								SetNonBlockingMode(bool inShouldBeNonBlocking);
	void							CleanupSocket();
private:
	friend class SocketUtil;
	TCPSocket( SOCKET inSocket ) : mSocket( inSocket ) {}
	SOCKET		mSocket;
};
typedef shared_ptr< TCPSocket > TCPSocketPtr;
