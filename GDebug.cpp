/*------------------------------------------------

  GDebug.cpp

	Runtine Debug functions


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GDebug.h"
#include "GWin32.h"
#include <float.h>
#include <mmsystem.h>



//	globals
//------------------------------------------------
Bool	g_GotConsole = FALSE;
HANDLE	g_ConsoleHandle = INVALID_HANDLE_VALUE;
int		g_ConsoleWidth = 80;	//	characters per line

const char*	GDebug::g_pLastBreakFile	= NULL;
const char*	GDebug::g_pLastClass		= NULL;
int			GDebug::g_LastBreakLine		= -1;


//	Definitions
//------------------------------------------------



Bool GDebug::Init()
{
	return TRUE;
}


Bool GDebug::BreakPrompt(GString& ErrStr, const char* pSrcFile, int LineNo )
{
	//	construct final string for prompt
	GString ErrorMessage;

	const char* pLineNoStr = (LineNo!=-1) ? ExportStringf("%d",LineNo) : "???";

	ErrorMessage.Append("Error in ");
	ErrorMessage.Append( pSrcFile ? pSrcFile : "Unknown file" );
	ErrorMessage.Append("(");
	ErrorMessage.Append( pLineNoStr );
	ErrorMessage.Append(") :: ");
	ErrorMessage.Append( g_pLastClass ? ExportStringf("In class \"%s\"", g_pLastClass ) : "In global scope" );
	ErrorMessage.Append(";\n");
	ErrorMessage.Append( ErrStr );
	
	//	print message to console first
	GDebug_Print( (char*)ErrorMessage );

	ErrorMessage.Append("\nPress Abort to try and quit the app, Retry to break, or Ignore to try and continue." );

	//	popup break options
	switch ( GWin32::Popup( Popup_AbortRetryIgnore, "Error!", (char*)ErrorMessage ) )
	{
		case PopupResult_OK:	//	retry
			//	break
			DebugBreak();
			return FALSE;
			break;

		case PopupResult_Cancel:	//	abort
			//	quit app
			if ( g_App )
			{
				g_App->m_AppFlags |= GAppFlags::QuitApp;
			}
			return FALSE;
			break;

		case PopupResult_Ignore:	//	ignore
			//	do nothing and continue past breakpoint
			return TRUE;
			break;
	}

	return FALSE;
}




Bool GDebug::Break(const char* pText, ... )
{
	char TxtOut[MAX_ERR_LEN];
	if ( strlen( pText ) >= MAX_ERR_LEN )
		return Break("Formatted debug string too long");

	va_list v;
	va_start( v, pText );
	int iChars = vsprintf((char*)TxtOut,pText, v );

	//	handle break
	return BreakPrompt( GString(TxtOut), GDebug::g_pLastBreakFile, GDebug::g_LastBreakLine );
}


Bool GDebug::Break(GString& String)
{
	//	handle break
	return BreakPrompt( String, GDebug::g_pLastBreakFile, GDebug::g_LastBreakLine );
}



void GDebug::Print(char* pText)
{
	if ( !pText )
		return;

	//	print out to console
	if ( g_GotConsole )
	{
		SetConsoleActiveScreenBuffer( g_ConsoleHandle );
		SetConsoleTextAttribute( g_ConsoleHandle, FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED );

		unsigned long CharsWritten=0;
		int StringLen = strlen( pText );
		WriteConsole( g_ConsoleHandle, pText, StringLen, &CharsWritten, NULL );
	}

	//	print to error stream regardless
	fprintf( stderr, pText );
}



void GDebug::Print(const char* pText,...)
{
	char TxtOut[MAX_ERR_LEN];
	if ( strlen( pText ) >= MAX_ERR_LEN )
		Break("Formatted debug string too long");

	va_list v;
	va_start( v, pText );
	int iChars = vsprintf((char*)TxtOut,pText, v );
	Print( TxtOut );
}





void GDebug::Show()
{
	if ( !g_GotConsole )
	{
		g_GotConsole = AllocConsole();

		if ( !g_GotConsole )
		{
			CheckWin32Error();
			return;
		}
	}
	
	u32 Access		= GENERIC_WRITE;//GENERIC_READ|GENERIC_WRITE;
	u32 ShareAccess	= 0;
	u32 Flags		= CONSOLE_TEXTMODE_BUFFER;

	g_ConsoleHandle = CreateConsoleScreenBuffer( Access, ShareAccess, NULL, CONSOLE_TEXTMODE_BUFFER, NULL );

	if ( g_ConsoleHandle == INVALID_HANDLE_VALUE )
	{
		CheckWin32Error();
		Break("Failed to make console buffer\n");
		return;
	}

	SetConsoleActiveScreenBuffer( g_ConsoleHandle );
	SetConsoleTitle("Debug");
	COORD LinesCols;
	LinesCols.X = g_ConsoleWidth;
	LinesCols.Y = 200;
	SetConsoleScreenBufferSize( g_ConsoleHandle, LinesCols );
}


void GDebug::Hide()
{
	if ( !g_GotConsole )
		return;

	if ( ! FreeConsole() )
	{
		CheckWin32Error();
		Break("Failed to free console\n");
	}
	g_ConsoleHandle = NULL;
	g_GotConsole = FALSE;

}


void GDebug::Shutdown()
{
	Hide();
}


#ifdef ENABLE_DEBUG
void GDebug::CheckGLError()
{
	if ( !GDisplay::g_OpenglInitialised )
		return;

	//	check for a GLError
	u16 Error = glGetError();

	//	no error
	if ( Error == GL_NO_ERROR )
		return;

	//	get error index
	Error ++;
	Error &= 0xff;

	const char* g_GLErrorUnknown = "GL_????: Unknown error type";
	const char* g_GLErrorMessages[] = 
	{
	//	"GL_NO_ERROR: No error has been recorded.",
		"GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.",
		"GL_INVALID_VALUE: A numeric argument is out of range.",
		"GL_INVALID_OPERATION: The specified operation is not allowed in the current state.",
		"GL_STACK_OVERFLOW: This command would cause a stack overflow.",
		"GL_STACK_UNDERFLOW: This command would cause a stack underflow.",
		"GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.",
		"GL_TABLE_TOO_LARGE: The specified table exceeds the implementation's maximum supported table size.",
	};

	if ( Error >= ARRAY_SIZE(g_GLErrorMessages) )
	{
		Break(g_GLErrorUnknown);
	}
	else
	{
		Break( g_GLErrorMessages[Error] );
	}

}
#endif

#ifdef ENABLE_DEBUG
void GDebug::CheckWin32Error()
{
	//	print out the last error from windows
	u32 Error = GetLastError();

	//	not a real error
	if ( Error == ERROR_SUCCESS )
		return;

	//	check code against
	//	http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/system_error_codes.asp

	//	get error string from windows
	char ErrorBuffer[MAX_ERR_LEN] = {0};
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, &Error, Error, 0, ErrorBuffer, MAX_ERR_LEN, NULL );

	Break("Win32 Last Error: [%d] %s\n", Error, ErrorBuffer );
}
#endif

#ifdef ENABLE_DEBUG
void GDebug::CheckMMSystemError(int ErrorCode)
{
	//	not an error
	if ( ErrorCode == MMSYSERR_NOERROR )
		return;

	//	
	const char* g_MMSystemErrors[] = 
	{
		"no error",
		"unspecified error",
		"device ID out of range",
		"driver failed enable",
		"device already allocated",
		"device handle is invalid",
		"no device driver present",
		"memory allocation error",
		"function isn't supported",
		"error value out of range",
		"invalid flag passed",
		"invalid parameter passed",
		"handle being used simultaneously on another thread (eg callback)",
		"specified alias not found",
		"bad registry database",
		"registry key not found",
		"registry read error",
 		"registry write error",
		"registry delete error",
		"registry value not found",
		"driver does not call DriverCallback",
		"more data to be returned",
	};

	const char* g_WAVEErrors[] = 
	{
		"unsupported wave format",
		"still something playing",
		"header not prepared",
		"device is synchronous",
	};


	if ( ErrorCode >= MMSYSERR_NOERROR && ErrorCode < MMSYSERR_LASTERROR )
	{
		Break("MMSystem Error: [%d] %s\n", ErrorCode, g_MMSystemErrors[ErrorCode - MMSYSERR_BASE] );
	}
	else if ( ErrorCode >= WAVERR_BASE && ErrorCode < WAVERR_LASTERROR )
	{
		Break("MMSystem(WAV) Error: [%d] %s\n", ErrorCode, g_WAVEErrors[ErrorCode - WAVERR_BASE] );
	}
	else
	{
		Break("MMSystem Error: [%d] Unknown error code\n", ErrorCode );
	}
}
#endif

#ifdef ENABLE_DEBUG
void GDebug::CheckFloat(float f)
{
	//	winapi float checks
	
	if ( _isnan( f ) )
	{
		GDebug::Break("Float %3.3f is a NaN\n", f );
	}
	
	if ( !_finite( f ) )
	{
		GDebug::Break("Float %3.3f is infinate\n", f );
	}
}
#endif


#ifdef ENABLE_DEBUG
void GDebug::CheckIndex(int Index,int Min,int Max)
{
	if ( Index < Min || Index >= Max )
	{
		GDebug::Break("Index out of bounds. %d >= %d < %d\n", Min, Index, Max );
	}
}
#endif


#ifdef ENABLE_DEBUG
void GDebug::CheckWinsockError()
{
	int WinsockError = WSAGetLastError();
	if ( WinsockError == 0 )
		return;

	typedef struct
	{
		int			Error;
		const char*	pErrString;
	
	} GWinsockErrorCode;

	const GWinsockErrorCode g_WinsockErrorCodes[] = 
	{
		{	WSABASEERR,			"Base error"	},
		{	WSAEINTR,			"interrupted system call"	},
		{	WSAEBADF,			"bad file number"	},
		{	WSAEACCES,			"permission denied "	},
		{	WSAEFAULT,			"bad address "	},
		{	WSAEINVAL,			"invalid argument "	},
		{	WSAEMFILE,			"too many open files "	},

	/*
	 * Windows Sockets definitions of regular Berkeley error constants
	 */
		{	WSAEWOULDBLOCK,		"operation would block "	},
		{	WSAEINPROGRESS,		"operation now in progress "	},
		{	WSAEALREADY,		"operation already in progress "	},
		{	WSAENOTSOCK,		"socket operation on non-socket "	},
		{	WSAEDESTADDRREQ,	"destination address required "	},
		{	WSAEMSGSIZE,		"message too long "	},
		{	WSAEPROTOTYPE,		"protocol wrong type for socket "	},
		{	WSAENOPROTOOPT,		"bad protocol option / protocol not available "	},
		{	WSAEPROTONOSUPPORT,	"protocol not suppported "	},
		{	WSAESOCKTNOSUPPORT,	"socket type not supported "	},
		{	WSAEOPNOTSUPP,		"operation not supported on socket "	},
		{	WSAEPFNOSUPPORT,	"protocol family not supported "	},
		{	WSAEAFNOSUPPORT,	"address family not supported by protocol family "	},
		{	WSAEADDRINUSE,		"address already in use "	},
		{	WSAEADDRNOTAVAIL,	"can't assign requested address "	},
		{	WSAENETDOWN,		"network is down "	},
		{	WSAENETUNREACH,		"network is unreachable "	},
		{	WSAENETRESET,		"network dropped connection on reset "	},
		{	WSAECONNABORTED,	"software caused connection abort "	},
		{	WSAECONNRESET,		"connection reset by peer "	},
		{	WSAENOBUFS,			"no buffer space available "	},
		{	WSAEISCONN,			"socket is already connected "	},
		{	WSAENOTCONN,		"socket is not connected "	},
		{	WSAESHUTDOWN,		"can't send after socket shutdown "	},
		{	WSAETOOMANYREFS,	"too many references can't splice "	},
		{	WSAETIMEDOUT,		"connection timed out "	},
		{	WSAECONNREFUSED,	"connection refused "	},
		{	WSAELOOP,			"too many levels of symbolic links "	},
		{	WSAENAMETOOLONG,	"file name too long "	},
		{	WSAEHOSTDOWN,		"host is down "	},
		{	WSAEHOSTUNREACH,	"no route to host "	},
		{	WSAENOTEMPTY,		"directory not empty "	},
		{	WSAEPROCLIM,		"too many processes "	},
		{	WSAEUSERS,			"too many users "	},
		{	WSAEDQUOT,			"disc quota exceeded "	},
		{	WSAESTALE,			"stale NFS file handle "	},
		{	WSAEREMOTE,			"too many levels of remote in path "	},
	//	{ WSAEDISCON,			""	},

	/*
	 * Extended Windows Sockets error constant definitions
	 */
		{	WSASYSNOTREADY,		"network subsystem is unavailable "	},
		{	WSAVERNOTSUPPORTED,	"winsock.dll version out of range "	},
		{	WSANOTINITIALISED,	"successful WSASTARTUP not yet performed"	},
	
		{	WSAHOST_NOT_FOUND,	"host not found"	},
		{	WSATRY_AGAIN,		"non-authoritative host not found"	},
		{	WSANO_RECOVERY,		"non-recoverable error"	},
		{	WSANO_DATA,			"valid name, no data record for requested name"	},
	};

	const char* pWinsockError = "Unknown winsock error";

	for ( int i=0;	i<ARRAY_SIZE(g_WinsockErrorCodes);	i++ )
	{
		if ( g_WinsockErrorCodes[i].Error == WinsockError )
		{
			pWinsockError = g_WinsockErrorCodes[i].pErrString;
			break;
		}
	}

	Break("Winsock Last Error: [%d] %s\n", WinsockError, pWinsockError );
}
#endif

