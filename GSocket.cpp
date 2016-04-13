/*------------------------------------------------

  GSocket.cpp

	Base winsock handling class


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GSocket.h"


//	globals
//------------------------------------------------
#define TCP	//	else UDP


//	Definitions
//------------------------------------------------


void GAddr::Debug(const char* Name)
{
	GDebug::Print("IP Address(%s): %03d.%03d.%03d.%03d (0x%08x)\n", Name?Name:"", IP[0],IP[1],IP[2],IP[3], Addr );
}




GSocket::GSocket()
{
	//	init vars
	m_Addr.Reset();
	m_Port = INVALID_PORT;

	m_Status = GSocketStatus_Unknown;
	
	m_ServerAddr.Reset();
	m_ServerPort = INVALID_PORT;
}


GSocket::~GSocket()
{
	//	try to disconnect
	Disconnect();
}


Bool GSocket::Init(u16 ListenPort,u32 SocketMessage)
{
	//	already connected or initialised, abort
	if ( m_Status != GSocketStatus_Unknown )
	{
		GDebug_Break("Socket already initialised\n");
		return FALSE;
	}

	WORD VersionRequested = MAKEWORD(2,2);
	WSAStartup( VersionRequested, &m_WSData );

	if ( m_WSData.wVersion != VersionRequested )
	{
		GDebug::Print("Invalid winsock version. %d.%d, expected %d.%d\n", LOBYTE(m_WSData.wVersion), HIBYTE(m_WSData.wVersion), LOBYTE(VersionRequested), HIBYTE(VersionRequested) );
		return FALSE;
	}


	//	grab hostname
	char HostName[256] = {0};
	gethostname( HostName, 256 );
	GDebug::Print("Initialising winsock for host \"%s\"\n", HostName );

	//	setup socket
	#ifdef TCP
	{
		m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		
	}
	#else // UDP
	{
		m_Socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	}
	#endif
	

	if ( m_Socket == INVALID_SOCKET )
	{
		#ifdef TCP
		{
			GDebug::Print("Failed to set TCP socket\n");
		}
		#else // UDP
		{
			GDebug::Print("Failed to set UDP socket\n");
		}
		#endif
	
		GDebug::CheckWinsockError();
		return FALSE;
	}

	//	set socket message
	m_SocketMessage = SocketMessage;

	//	set port
	m_Port = ListenPort;

	//	init local address (will be set after we bind)
	m_Addr.Reset();

	//	setup winsock to listen
	m_SocketAddrIn.sin_family		= AF_INET;
	m_SocketAddrIn.sin_addr.s_addr	= INADDR_ANY;	//	winsock assigns its address
//	m_SocketAddrIn.sin_port			= htons( m_Port );
	m_SocketAddrIn.sin_port			= m_Port;

	if ( bind( m_Socket, (LPSOCKADDR)&m_SocketAddrIn, sizeof(struct sockaddr) ) == SOCKET_ERROR  )
	{
		GDebug::Print("Failed to bind socket to port %d", m_Port );
		GDebug::CheckWinsockError();
		return FALSE;
	}

	//	grab the ip address
	m_Addr.Addr = m_SocketAddrIn.sin_addr.S_un.S_addr;
	m_Addr.Debug("Local");

	//	setup windows message for reciveing packets
	WSAAsyncSelect( m_Socket, GApp::g_pApp->Hwnd(), m_SocketMessage, FD_READ );

	m_Status = GSocketStatus_Disconnected;

	return TRUE;
}



GSocketResult GSocket::Connect( GAddr Address, u16 Port )
{
	//	check status before trying to connect
	if ( m_Status == GSocketStatus_Unknown )
	{
		GDebug_Break("Trying to connect with unintialised socket\n");
		return GSocketResult_Uninitialised;
	}

	if ( m_Status == GSocketStatus_Connected )
	{
		GDebug_Break("Trying to connect when already connected. Disconect first\n");
		return GSocketResult_AlreadyConnected;
	}

	#ifdef TCP
	{
		m_ServerAddr = Address;
		m_ServerPort = Port;

		LPHOSTENT pHostEntry = gethostbyaddr( (char*)(&m_ServerAddr.Addr), 4, AF_INET );

		if ( !pHostEntry )
		{
			GDebug::Print("Couldnt get host entry to connect\n");
			GDebug::CheckWinsockError();
			return GSocketResult_Error;
		}

		m_SocketAddrIn.sin_family	= AF_INET;
		m_SocketAddrIn.sin_port		= Port;
		m_SocketAddrIn.sin_addr.S_un.S_un_b.s_b1	= pHostEntry->h_addr_list[0][0];
		m_SocketAddrIn.sin_addr.S_un.S_un_b.s_b2	= pHostEntry->h_addr_list[0][1];
		m_SocketAddrIn.sin_addr.S_un.S_un_b.s_b3	= pHostEntry->h_addr_list[0][2];
		m_SocketAddrIn.sin_addr.S_un.S_un_b.s_b4	= pHostEntry->h_addr_list[0][3];

		int ConnectError = connect( m_Socket, (LPSOCKADDR)&m_SocketAddrIn, sizeof(struct sockaddr) );

		if ( ConnectError == SOCKET_ERROR )
		{
			GDebug::Print("Couldnt connect\n");
			GDebug::CheckWinsockError();
			return GSocketResult_Error;
		}

		m_Status = GSocketStatus_Connected;
		return GSocketResult_OK;

	}
	#else // UDP
	{
		//	udp doesnt connect. set the server address and set status to connected
		m_Status = GSocketStatus_Connected;
		m_ServerAddr = Address;
		m_ServerPort = Port;
		return GSocketResult_Okay;
	}
	#endif


	//	needs to be overloaded per protocol
	return GSocketResult_Error;
}



GSocketResult GSocket::Disconnect()
{
	//	not connected, cancel
	if ( m_Status != GSocketStatus_Connected )
	{
		GDebug::Print("Socket not connected. Cannot disconnect\n");
		return GSocketResult_NotConnected;
	}


	return GSocketResult_OK;
}



Bool GSocket::Update()
{
	return TRUE;
}



