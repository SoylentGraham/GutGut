/*------------------------------------------------

  GDisplayExt.cpp

	Extension


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GDisplayExt.h"
#include "GDisplay.h"
#include "GShader.h"


//	globals
//------------------------------------------------

//	setup flags for hardware that will never be availible (for debugging etc)
const u32	g_ForceUnsupportedHardware = GHardwareFlags::VertexBufferObjects;


const char* g_HardwareExtensionNames[GHardware_Max] = 
{
	"GL_ARB_multitexture",			//	GHardware_MultiTexturing
	"GL_ARB_vertex_buffer_object",	//	GHardware_VertexBufferObjects
	"glDrawRangeElements",			//	(function) GHardware_DrawRangeElements
	"wglSwapIntervalEXT",			//	(function) GHardware_SwapInterval
	"GL_NV_vertex_program",			//	GHardware_NVVertexProgram
	"GL_ARB_vertex_program",		//	GHardware_ARBVertexProgram
	"GL_ARB_fragment_program",		//	ARBFragmentProgram
};


//	Definitions
//------------------------------------------------

GDisplayExt::GDisplayExt()
{
	m_DisabledHardwareFlags = 0x0;
	m_HardwareFlags			= 0x0;
}

//-------------------------------------------------------------------------
//	find a function address and add it to the list for this hardware extension
//-------------------------------------------------------------------------
Bool GDisplayExt::AddExtensionFunction(const char* pFunctionName, int HardwareIndex )
{
	void* pAddr = wglGetProcAddress(pFunctionName);
	
	if ( !pAddr )
	{
		GDebug_Break("Address %s missing on extension %s, extension disabled\n", pFunctionName, g_HardwareExtensionNames[HardwareIndex] );
		m_ExtensionAddresses[HardwareIndex].Empty();
		return FALSE;
	}

	m_ExtensionAddresses[HardwareIndex].Add( pAddr );
	return TRUE;
}




//-------------------------------------------------------------------------
//	initialises all extensions
//-------------------------------------------------------------------------
Bool GDisplayExt::Init()
{
	//	check hardware support
	m_HardwareFlags = 0x0;

	if ( GDisplay::g_OpenglInitialised )
	{
		for ( int i=0;	i<GHardware_Max;	i++ )
		{
			//	skip forced-disabled hardware
			if ( g_ForceUnsupportedHardware & (1<<i) )
				continue;

			//	check extension supported
			if ( ExtensionSupported( g_HardwareExtensionNames[i] ) )
			{
				m_HardwareFlags |= 1<<(i);

				GDebug::Print("%s extension supported\n", g_HardwareExtensionNames[i] );
			}
			else
			{
				//	check for functions
				if ( wglGetProcAddress( g_HardwareExtensionNames[i] ) )
				{
					m_HardwareFlags |= 1<<(i);
					GDebug::Print("%s function supported\n", g_HardwareExtensionNames[i] );
				}
			}			

			//	reset addresses
			m_ExtensionAddresses[i].Empty();
		}

		if ( m_HardwareFlags != 0x0 )
			GDebug::Print("-------------\n");

		//	setup hardware extensions
		#define ADDEXT_FUNC(func,hardware)						\
		{														\
			if ( m_HardwareFlags & (1<<(hardware)) )			\
				if ( !AddExtensionFunction( func, hardware ) )	\
					m_HardwareFlags &= ~(1<<(hardware));		\
		}														\


		//	multitexturing
		if ( m_HardwareFlags & GHardwareFlags::MultiTexturing )
		{
			ADDEXT_FUNC( "glActiveTextureARB",			GHardware_MultiTexturing );
			ADDEXT_FUNC( "glMultiTexCoord2fARB",		GHardware_MultiTexturing );
			ADDEXT_FUNC( "glClientActiveTextureARB",	GHardware_MultiTexturing );
		}

		//	vertex buffer objects
		if ( m_HardwareFlags & GHardwareFlags::VertexBufferObjects )
		{
			ADDEXT_FUNC( "glGenBuffersARB", GHardware_VertexBufferObjects );
			ADDEXT_FUNC( "glBindBufferARB", GHardware_VertexBufferObjects );
			ADDEXT_FUNC( "glBufferDataARB", GHardware_VertexBufferObjects );
			ADDEXT_FUNC( "glDeleteBuffersARB", GHardware_VertexBufferObjects );
		}

		//	draw range elements
		if ( m_HardwareFlags & GHardwareFlags::DrawRangeElements )
		{
			ADDEXT_FUNC( g_HardwareExtensionNames[GHardware_DrawRangeElements], GHardware_DrawRangeElements );
		}

		//	swap interval
		if ( m_HardwareFlags & GHardwareFlags::SwapInteral )
		{
			ADDEXT_FUNC( g_HardwareExtensionNames[GHardware_SwapInterval], GHardware_SwapInterval );
		}

		//	nv vertex program
		if ( m_HardwareFlags & GHardwareFlags::NVVertexProgram )
		{
			if ( !GShader::InitHardware( GHardware_NVVertexProgram ) )
			{
				//	failed to init, remove support
				m_HardwareFlags &= ~GHardwareFlags::NVVertexProgram;
			}
		}

		//	arb vertex program
		if ( m_HardwareFlags & GHardwareFlags::ARBVertexProgram )
		{
			if ( !GShader::InitHardware( GHardware_ARBVertexProgram ) )
			{
				//	failed to init, remove support
				m_HardwareFlags &= ~GHardwareFlags::ARBVertexProgram;
			}
		}

	}

	return TRUE;
}


//-------------------------------------------------------------------------
//	shutdown all extensions
//-------------------------------------------------------------------------
void GDisplayExt::Shutdown()
{
}


//-------------------------------------------------------------------------
//	check if a string (extension name) is in the list of supported extensions
//-------------------------------------------------------------------------
Bool GDisplayExt::ExtensionSupported(const char* pExtensionName)
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *) strchr( pExtensionName, ' ' );
	if( pszWhere || *pExtensionName == '\0' )
		return FALSE;

	// Get Extensions String
	pszExtensions = glGetString( GL_EXTENSIONS );
	if ( ! pszExtensions )
		return FALSE;

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for(;;)
	{
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, pExtensionName );
		if( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( pExtensionName );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
			if( *pszTerminator == ' ' || *pszTerminator == '\0' )
				return TRUE;
		pszStart = pszTerminator;
	}
	return FALSE;
}
