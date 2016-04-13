/*------------------------------------------------

  GSocket Header file



-------------------------------------------------*/

#ifndef __GSOCKET__H_
#define __GSOCKET__H_



//	Includes
//------------------------------------------------
#include "GWin32.h"



//	Macros
//------------------------------------------------
#define INVALID_PORT	0xffff

typedef enum GSocketStatus
{
	GSocketStatus_Unknown,
	GSocketStatus_Connected,
	GSocketStatus_Disconnected,
};

typedef enum GSocketResult
{
	GSocketResult_OK,
	GSocketResult_Uninitialised,
	GSocketResult_AlreadyConnected,
	GSocketResult_NotConnected,
	GSocketResult_Error,
};


//	Types
//------------------------------------------------

/*
	IP address. host byte order (127 0 0 1)
*/
typedef struct
{
	union
	{
		u32	Addr;
		u8	IP[4];
	};

	inline void	Set(u8 a, u8 b, u8 c, u8 d)	{	IP[0]=a;	IP[1]=b;	IP[2]=c;	IP[3]=d;	};
	inline void	Reset()						{	Addr = 0;	};
	void		Debug(const char* Name=NULL);

} GAddr;





/*
	base network socket type
*/
class GSocket
{
public:
	GAddr			m_Addr;		//	our address
	u16				m_Port;		//	our(client) listening port
	GSocketStatus	m_Status;
	
	GAddr			m_ServerAddr;	//	host (server) address
	u16				m_ServerPort;	//	host port

	WSADATA			m_WSData;
	SOCKET			m_Socket;
	SOCKADDR_IN		m_SocketAddrIn;

	u32				m_SocketMessage;	//	when the app recieves this message, its for this socket

public:
	GSocket();
	~GSocket();

	virtual Bool			Init(u16 ListenPort,u32 SocketMessage);	//	init socket. Which port we're listening on, and what message the app should receieve
	virtual GSocketResult	Connect(GAddr Address, u16 Port);		//	starts a connection with a host
	virtual GSocketResult	Disconnect();							//	disconect from host
	virtual Bool			Update();								//	update 

};




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------



#endif

