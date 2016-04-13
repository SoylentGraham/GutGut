/*------------------------------------------------

  GShader.cpp

	Vertex shader classes


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GShader.h"
#include "GDebug.h"
#include "GDisplay.h"


//	globals
//------------------------------------------------


//	Definitions
//------------------------------------------------

//--------------------------------------------------------------------------------------------------------
// init hardware for shaders
//--------------------------------------------------------------------------------------------------------
/*static*/Bool GShader::InitHardware(int HardwareType)
{
	#define ADD_FUNC(funcname)							\
	{													\
		if ( !g_DisplayExt.AddExtensionFunction( funcname, HardwareType ) )	\
			return FALSE;								\
	}													\

	if ( HardwareType == GHardware_NVVertexProgram )
	{
		ADD_FUNC("glAreProgramsResidentNV");
		ADD_FUNC("glBindProgramNV");
		ADD_FUNC("glDeleteProgramsNV");
		ADD_FUNC("glExecuteProgramNV");
		ADD_FUNC("glGenProgramsNV");
		ADD_FUNC("glGetProgramParameterdvNV");
		ADD_FUNC("glGetProgramParameterfvNV");
		ADD_FUNC("glGetProgramivNV");
		ADD_FUNC("glGetProgramStringNV");
		ADD_FUNC("glGetTrackMatrixivNV");
		ADD_FUNC("glGetVertexAttribdvNV");
		ADD_FUNC("glGetVertexAttribfvNV");
		ADD_FUNC("glGetVertexAttribivNV");
		ADD_FUNC("glGetVertexAttribPointervNV");
		ADD_FUNC("glIsProgramNV");
		ADD_FUNC("glLoadProgramNV");
		ADD_FUNC("glProgramParameter4dNV");
		ADD_FUNC("glProgramParameters4dvNV");
		ADD_FUNC("glProgramParameter4fNV");
		ADD_FUNC("glProgramParameter4fvNV");
		ADD_FUNC("glRequestResidentProgramsNV");
		ADD_FUNC("glTrackMatrixNV");
		ADD_FUNC("glVertexAttribPointerNV");
		ADD_FUNC("glVertexAttrib1dNV");
		ADD_FUNC("glVertexAttrib1dvNV");
		ADD_FUNC("glVertexAttrib1fNV");
		ADD_FUNC("glVertexAttrib1fvNV");
		ADD_FUNC("glVertexAttrib1sNV");
		ADD_FUNC("glVertexAttrib1svNV");
		ADD_FUNC("glVertexAttrib2dNV");
		ADD_FUNC("glVertexAttrib2dvNV");
		ADD_FUNC("glVertexAttrib2fNV");
		ADD_FUNC("glVertexAttrib2fvNV");
		ADD_FUNC("glVertexAttrib2sNV");
		ADD_FUNC("glVertexAttrib2svNV");
		ADD_FUNC("glVertexAttrib3dNV");
		ADD_FUNC("glVertexAttrib3dvNV");
		ADD_FUNC("glVertexAttrib3fNV");
		ADD_FUNC("glVertexAttrib3fvNV");
		ADD_FUNC("glVertexAttrib3sNV");
		ADD_FUNC("glVertexAttrib3svNV");
		ADD_FUNC("glVertexAttrib4dNV");
		ADD_FUNC("glVertexAttrib4dvNV");
		ADD_FUNC("glVertexAttrib4fNV");
		ADD_FUNC("glVertexAttrib4fvNV");
		ADD_FUNC("glVertexAttrib4sNV");
		ADD_FUNC("glVertexAttrib4svNV");
		ADD_FUNC("glVertexAttrib4ubvNV");
		ADD_FUNC("glVertexAttribs1dvNV");
		ADD_FUNC("glVertexAttribs1fvNV");
		ADD_FUNC("glVertexAttribs1svNV");
		ADD_FUNC("glVertexAttribs2dvNV");
		ADD_FUNC("glVertexAttribs2fvNV");
		ADD_FUNC("glVertexAttribs2svNV");
		ADD_FUNC("glVertexAttribs3dvNV");
		ADD_FUNC("glVertexAttribs3fvNV");
		ADD_FUNC("glVertexAttribs3svNV");
		ADD_FUNC("glVertexAttribs4dvNV");
		ADD_FUNC("glVertexAttribs4fvNV");
		ADD_FUNC("glVertexAttribs4svNV");
		ADD_FUNC("glVertexAttribs4ubvNV");
	}

	if ( HardwareType == GHardware_ARBVertexProgram )
	{
		ADD_FUNC("glVertexAttrib1sARB");	//	PFNGLVERTEXATTRIB1SARB
		ADD_FUNC("glVertexAttrib1fARB");	//	PFNGLVERTEXATTRIB1FARB
		ADD_FUNC("glVertexAttrib1dARB");	//	PFNGLVERTEXATTRIB1DARB
		ADD_FUNC("glVertexAttrib2sARB");	//	PFNGLVERTEXATTRIB2SARB
		ADD_FUNC("glVertexAttrib2fARB");	//	PFNGLVERTEXATTRIB2FARB
		ADD_FUNC("glVertexAttrib2dARB");	//	PFNGLVERTEXATTRIB2DARB
		ADD_FUNC("glVertexAttrib3sARB");	//	PFNGLVERTEXATTRIB3SARB
		ADD_FUNC("glVertexAttrib3fARB");	//	PFNGLVERTEXATTRIB3FARB
		ADD_FUNC("glVertexAttrib3dARB");	//	PFNGLVERTEXATTRIB3DARB
		ADD_FUNC("glVertexAttrib4sARB");	//	PFNGLVERTEXATTRIB4SARB
		ADD_FUNC("glVertexAttrib4fARB");	//	PFNGLVERTEXATTRIB4FARB
		ADD_FUNC("glVertexAttrib4dARB");	//	PFNGLVERTEXATTRIB4DARB
		ADD_FUNC("glVertexAttrib4NubARB");	//	PFNGLVERTEXATTRIB4NUBARB
		ADD_FUNC("glVertexAttrib1svARB");	//	PFNGLVERTEXATTRIB1SVARB
		ADD_FUNC("glVertexAttrib1fvARB");	//	PFNGLVERTEXATTRIB1FVARB
		ADD_FUNC("glVertexAttrib1dvARB");	//	PFNGLVERTEXATTRIB1DVARB
		ADD_FUNC("glVertexAttrib2svARB");	//	PFNGLVERTEXATTRIB2SVARB
		ADD_FUNC("glVertexAttrib2fvARB");	//	PFNGLVERTEXATTRIB2FVARB
		ADD_FUNC("glVertexAttrib2dvARB");	//	PFNGLVERTEXATTRIB2DVARB
		ADD_FUNC("glVertexAttrib3svARB");	//	PFNGLVERTEXATTRIB3SVARB
		ADD_FUNC("glVertexAttrib3fvARB");	//	PFNGLVERTEXATTRIB3FVARB
		ADD_FUNC("glVertexAttrib3dvARB");	//	PFNGLVERTEXATTRIB3DVARB
		ADD_FUNC("glVertexAttrib4bvARB");	//	PFNGLVERTEXATTRIB4BVARB
		ADD_FUNC("glVertexAttrib4svARB");	//	PFNGLVERTEXATTRIB4SVARB
		ADD_FUNC("glVertexAttrib4ivARB");	//	PFNGLVERTEXATTRIB4IVARB
		ADD_FUNC("glVertexAttrib4ubvARB");	//	PFNGLVERTEXATTRIB4UBVARB
		ADD_FUNC("glVertexAttrib4usvARB");	//	PFNGLVERTEXATTRIB4USVARB
		ADD_FUNC("glVertexAttrib4uivARB");	//	PFNGLVERTEXATTRIB4UIVARB
		ADD_FUNC("glVertexAttrib4fvARB");	//	PFNGLVERTEXATTRIB4FVARB
		ADD_FUNC("glVertexAttrib4dvARB");	//	PFNGLVERTEXATTRIB4DVARB
		ADD_FUNC("glVertexAttrib4NbvARB");	//	PFNGLVERTEXATTRIB4NBVARB
		ADD_FUNC("glVertexAttrib4NsvARB");	//	PFNGLVERTEXATTRIB4NSVARB
		ADD_FUNC("glVertexAttrib4NivARB");	//	PFNGLVERTEXATTRIB4NIVARB
		ADD_FUNC("glVertexAttrib4NubvARB");	//	PFNGLVERTEXATTRIB4NUBVARB
		ADD_FUNC("glVertexAttrib4NusvARB");	//	PFNGLVERTEXATTRIB4NUSVARB
		ADD_FUNC("glVertexAttrib4NuivARB");	//	PFNGLVERTEXATTRIB4NUIVARB
		ADD_FUNC("glVertexAttribPointerARB");	//	PFNGLVERTEXATTRIBPOINTERARB
		ADD_FUNC("glEnableVertexAttribArrayARB");	//	PFNGLENABLEVERTEXATTRIBARRAYARB
		ADD_FUNC("glDisableVertexAttribArrayARB");	//	PFNGLDISABLEVERTEXATTRIBARRAYARB
		ADD_FUNC("glProgramStringARB");	//	PFNGLPROGRAMSTRINGARB
		ADD_FUNC("glBindProgramARB");	//	PFNGLBINDPROGRAMARB
		ADD_FUNC("glDeleteProgramsARB");	//	PFNGLDELETEPROGRAMSARB
		ADD_FUNC("glGenProgramsARB");	//	PFNGLGENPROGRAMSARB
		ADD_FUNC("glProgramEnvParameter4dARB");	//	PFNGLPROGRAMENVPARAMETER4DARB
		ADD_FUNC("glProgramEnvParameter4dvARB");	//	PFNGLPROGRAMENVPARAMETER4DVARB
		ADD_FUNC("glProgramEnvParameter4fARB");	//	PFNGLPROGRAMENVPARAMETER4FARB
		ADD_FUNC("glProgramEnvParameter4fvARB");	//	PFNGLPROGRAMENVPARAMETER4FVARB
		ADD_FUNC("glProgramLocalParameter4dARB");	//	PFNGLPROGRAMLOCALPARAMETER4DARB
		ADD_FUNC("glProgramLocalParameter4dvARB");	//	PFNGLPROGRAMLOCALPARAMETER4DVARB
		ADD_FUNC("glProgramLocalParameter4fARB");	//	PFNGLPROGRAMLOCALPARAMETER4FARB
		ADD_FUNC("glProgramLocalParameter4fvARB");	//	PFNGLPROGRAMLOCALPARAMETER4FVARB
		ADD_FUNC("glGetProgramEnvParameterdvARB");	//	PFNGLGETPROGRAMENVPARAMETERDVARB
		ADD_FUNC("glGetProgramEnvParameterfvARB");	//	PFNGLGETPROGRAMENVPARAMETERFVARB
		ADD_FUNC("glGetProgramLocalParameterdvARB");	//	PFNGLGETPROGRAMLOCALPARAMETERDVARB
		ADD_FUNC("glGetProgramLocalParameterfvARB");	//	PFNGLGETPROGRAMLOCALPARAMETERFVARB
		ADD_FUNC("glGetProgramivARB");	//	PFNGLGETPROGRAMIVARB
		ADD_FUNC("glGetProgramStringARB");	//	PFNGLGETPROGRAMSTRINGARB
		ADD_FUNC("glGetVertexAttribdvARB");	//	PFNGLGETVERTEXATTRIBDVARB
		ADD_FUNC("glGetVertexAttribfvARB");	//	PFNGLGETVERTEXATTRIBFVARB
		ADD_FUNC("glGetVertexAttribivARB");	//	PFNGLGETVERTEXATTRIBIVARB
		ADD_FUNC("glGetVertexAttribPointervARB");	//	PFNGLGETVERTEXATTRIBPOINTERVARB
		ADD_FUNC("glIsProgramARB");	//	PFNGLISPROGRAMARB
	}


	
	return TRUE;
	#undef ADD_FUNC
}

//--------------------------------------------------------------------------------------------------------
// cleanup hardware for shaders
//--------------------------------------------------------------------------------------------------------
/*static*/void GShader::ShutdownHardware()
{
	GDebug::Print("Todo\n");
}


GShader::GShader()
{
	m_VertexProgramID	= 0;
	m_CurrentMode		= GShaderMode_None;
	m_pOwner			= NULL;
	m_FirstConstant		= 0;
}

GShader::~GShader()
{
	Deload();
}


//-------------------------------------------------------------------------
//	get what will be the active mode, ie. hardware if supported, or software, or none
//-------------------------------------------------------------------------
GShaderMode GShader::SetDrawMode()
{
	//	no shader availible?
	if ( !HardwareVersion() && !SoftwareVersion() )
	{
		return ( m_CurrentMode = GShaderMode_None );
	}

	//	use hardware
	if ( HardwareVersion() && IsLoaded() && HardwareSupported() )
	{
		return ( m_CurrentMode = GShaderMode_Hardware );
	}

	//	use software
	if ( SoftwareVersion() )
	{
		return ( m_CurrentMode = GShaderMode_Software );
	}

	return ( m_CurrentMode = GShaderMode_None );	
}
	


//--------------------------------------------------------------------------------------------------------
//	setup the hardware shader, if that fails, or we want to use the software shader anyway, execute the 
//	software shader
//--------------------------------------------------------------------------------------------------------
Bool GShader::PreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer)
{
	//	set draw mode if not set
	if ( m_CurrentMode == GShaderMode_None )
		SetDrawMode();

	//	no mode, fail
	if ( m_CurrentMode == GShaderMode_None )
		return FALSE;

	if ( m_CurrentMode == GShaderMode_Hardware )
	{
		//	enable & bind
		#ifdef USE_ARB_PROGRAM
			glEnable( GL_VERTEX_PROGRAM_ARB );
		    g_DisplayExt.glBindProgramARB()( GL_VERTEX_PROGRAM_ARB, m_VertexProgramID );
		#else
			glEnable( GL_VERTEX_PROGRAM_NV );
		    g_DisplayExt.glBindProgramNV()( GL_VERTEX_PROGRAM_NV, m_VertexProgramID );
		#endif
		GDebug::CheckGLError();

		m_CurrentMode = GShaderMode_Hardware;

		if ( HardwarePreDraw( pMesh, DrawInfo, pVertexBuffer, pNormalBuffer, pTextureUVBuffer, pTextureUV2Buffer, pColourBuffer ) )
		{
			return TRUE;
		}
		else
		{
			//	failed, disable program
			#ifdef USE_ARB_PROGRAM
				glDisable( GL_VERTEX_PROGRAM_ARB );
			#else
				glDisable( GL_VERTEX_PROGRAM_NV );
			#endif
			m_CurrentMode = GShaderMode_None;
			return FALSE;
		}

	}	

	
	//	execute software version
	if ( m_CurrentMode == GShaderMode_Software)
	{
		if ( SoftwarePreDraw( pMesh, DrawInfo, pVertexBuffer, pNormalBuffer, pTextureUVBuffer, pTextureUV2Buffer, pColourBuffer ) )
		{
			return TRUE;
		}
		else
		{
			m_CurrentMode = GShaderMode_None;
			return FALSE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------
//	after all the mesh geometry is drawn
//-------------------------------------------------------------------------
void GShader::PostDraw(GMesh* pMesh,GDrawInfo& DrawInfo)
{
	switch( m_CurrentMode )
	{
		case GShaderMode_None:
			break;

		case GShaderMode_Hardware:
			#ifdef USE_ARB_PROGRAM
				g_DisplayExt.glBindProgramARB()( GL_VERTEX_PROGRAM_ARB, 0 );
				glDisable( GL_VERTEX_PROGRAM_ARB );
			#else
				g_DisplayExt.glBindProgramNV()( GL_VERTEX_PROGRAM_NV, 0 );
				glDisable( GL_VERTEX_PROGRAM_NV );
			#endif
			break;

		case GShaderMode_Software:
			break;
	}
	
	//	reset mode
	m_CurrentMode = GShaderMode_None;
}


//-------------------------------------------------------------------------
//	fill the string with the program code
//-------------------------------------------------------------------------
Bool GShader::GetVertexProgram(GString& String)
{
	return FALSE;
}


Bool GShader::Upload()
{
	if ( !HardwareSupported() )
		return FALSE;

	if ( m_VertexProgramID != 0 )
	{
		GDebug_Break("Tried to upload shader thats already uploaded\n");
		return FALSE;
	}

	//	enable vertex program operations
	#ifdef USE_ARB_PROGRAM
		glEnable(GL_VERTEX_PROGRAM_ARB);
	#else
		glEnable(GL_VERTEX_PROGRAM_NV);
	#endif
	GDebug::CheckGLError();

	//	create new program entry
	#ifdef USE_ARB_PROGRAM
		g_DisplayExt.glGenProgramsARB()( 1, &m_VertexProgramID );
	#else
		g_DisplayExt.glGenProgramsNV()( 1, &m_VertexProgramID );
	#endif

	if ( m_VertexProgramID == 0 )
	{
		GDebug_Print("Failed to allocate vertex program\n");
		#ifdef USE_ARB_PROGRAM
			glDisable(GL_VERTEX_PROGRAM_ARB);
		#else
			glDisable(GL_VERTEX_PROGRAM_NV);
		#endif
		return FALSE;
	}
	
	//	bind
	#ifdef USE_ARB_PROGRAM
	    g_DisplayExt.glBindProgramARB()( GL_VERTEX_PROGRAM_ARB, m_VertexProgramID );
	#else
	    g_DisplayExt.glBindProgramNV()( GL_VERTEX_PROGRAM_NV, m_VertexProgramID );
	#endif
	GDebug::CheckGLError();

	//	get the vertex program code
	GString VertexProgram;
	if ( !GetVertexProgram( VertexProgram ) || VertexProgram.Length() == 0 )
	{
		GDebug_Print("Error uploading shader, missing vertex program\n");
		#ifdef USE_ARB_PROGRAM
			glDisable(GL_VERTEX_PROGRAM_ARB);
		#else
			glDisable(GL_VERTEX_PROGRAM_NV);
		#endif
		Deload();
		return FALSE;
	}
	
	//	load program
	const u8* pProgram = (const u8*)VertexProgram.Data();
	#ifdef USE_ARB_PROGRAM
		g_DisplayExt.glProgramStringARB()( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, VertexProgram.Length(), pProgram );
	#else
		g_DisplayExt.glLoadProgramNV()( GL_VERTEX_PROGRAM_NV, m_VertexProgramID, VertexProgram.Length(), pProgram );
	#endif
	//GDebug::CheckGLError();

	//	check for errors
	int ErrorPos = -1;
	#ifdef USE_ARB_PROGRAM
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &ErrorPos );
	#else
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &ErrorPos );
	#endif
	//GDebug::CheckGLError();
	if (ErrorPos != -1 )
	{
		GString Error;
		int bgn = ErrorPos - 10;
		bgn = bgn < 0 ? 0 : bgn;
		char* c = (char*)( pProgram + bgn );
		for(int i = 0; i < 30; i++)
		{
			if(bgn+i >= int(VertexProgram.Length()-1))
				break;
			Error += *c++;
		}
		GDebug_Print("Some vertex shader loading(syntax) error...\n");
		GDebug_Print( Error.Export() );
		GDebug_Print("\n");

		//	get arb error
		#ifdef USE_ARB_PROGRAM
			const char* pArbErr = (const char*)glGetString( GL_PROGRAM_ERROR_STRING_ARB );
			GDebug_Break("ARB Error: %s\n", pArbErr );
		#endif
		
		#ifdef USE_ARB_PROGRAM
			glDisable(GL_VERTEX_PROGRAM_ARB);
		#else
			glDisable(GL_VERTEX_PROGRAM_NV);
		#endif

		Deload();
		return FALSE;
	}


	
	#ifdef USE_ARB_PROGRAM
	
		//	projection matrixes are already in constants
		m_FirstConstant = 8;

	#else
		//	put projection matrix into registers from 0 (so 0,1,2,3)
		g_DisplayExt.glTrackMatrixNV()( GL_VERTEX_PROGRAM_NV, m_FirstConstant, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV );
		m_FirstConstant += 4;
		GDebug::CheckGLError();
	#endif
	
	//	finished uploading
	#ifdef USE_ARB_PROGRAM
		glDisable(GL_VERTEX_PROGRAM_ARB);
	#else
		glDisable(GL_VERTEX_PROGRAM_NV);
	#endif
	GDebug::CheckGLError();

	return TRUE;
}


Bool GShader::Deload()
{
	if ( !HardwareSupported() )
		return FALSE;

	//	already deloaded
	if ( m_VertexProgramID == 0 )
		return TRUE;

	//	delete program
	#ifdef USE_ARB_PROGRAM
		glEnable(GL_VERTEX_PROGRAM_ARB);
		g_DisplayExt.glDeleteProgramsARB()( 1, &m_VertexProgramID );
		glDisable(GL_VERTEX_PROGRAM_ARB);
	#else
		glEnable(GL_VERTEX_PROGRAM_NV);
		g_DisplayExt.glDeleteProgramsNV()( 1, &m_VertexProgramID );
		glDisable(GL_VERTEX_PROGRAM_NV);
	#endif

	m_VertexProgramID = 0;
	GDebug::CheckGLError();

	return TRUE;
}


void GShader::LoadConstant(int ConstantIndex, float4& xyzw )
{
	//	check for invalid index
	if ( ConstantIndex < 0 )
	{
		GDebug::CheckIndex( ConstantIndex, 0, MAX_CONSTANTS );
		return;
	}

	if ( ConstantIndex >= MAX_CONSTANTS )
	{
		GDebug_Print("Invalid shader constant index %d. max %d\n", ConstantIndex, MAX_CONSTANTS );
		return;
	}

	#ifdef USE_ARB_PROGRAM
		g_DisplayExt.glProgramLocalParameter4fvARB()( GL_VERTEX_PROGRAM_NV, ConstantIndex, xyzw );
	#else
		g_DisplayExt.glProgramParameter4fvNV()( GL_VERTEX_PROGRAM_NV, ConstantIndex, xyzw );
	#endif
}



//////////////////////////////////////////////////////////////////////////


GPixelShader::GPixelShader()
{
	m_FragmentProgramID	= 0;
	m_pOwner			= NULL;
}

GPixelShader::~GPixelShader()
{
	Deload();
}


//--------------------------------------------------------------------------------------------------------
//	setup the hardware shader
//--------------------------------------------------------------------------------------------------------
Bool GPixelShader::PreDraw(GMesh* pMesh,GDrawInfo& DrawInfo, GList<float3>*& pVertexBuffer, GList<float3>*& pNormalBuffer, GList<float2>*& pTextureUVBuffer, GList<float2>*& pTextureUV2Buffer, GList<float3>*& pColourBuffer)
{
	//	no program, assert
	if ( !IsLoaded() )
	{
		GDebug_Break("Executing pixel shader which hasnt been uploaded\n");
		return FALSE;
	}

	//	enable & bind
	glEnable( GL_FRAGMENT_PROGRAM_ARB );
    g_DisplayExt.glBindProgramARB()( GL_FRAGMENT_PROGRAM_ARB, m_FragmentProgramID );
	GDebug::CheckGLError();

	if ( !HardwarePreDraw( pMesh, DrawInfo, pVertexBuffer, pNormalBuffer, pTextureUVBuffer, pTextureUV2Buffer, pColourBuffer ) )
	{
		//	failed, disable program
		glDisable( GL_FRAGMENT_PROGRAM_ARB );
		return FALSE;
	}	

	return TRUE;
}

//-------------------------------------------------------------------------
//	after all the mesh geometry is drawn
//-------------------------------------------------------------------------
void GPixelShader::PostDraw(GMesh* pMesh,GDrawInfo& DrawInfo)
{
	//todo:	no check for pre-binding....
	g_DisplayExt.glBindProgramARB()( GL_FRAGMENT_PROGRAM_ARB, 0 );
	glDisable( GL_FRAGMENT_PROGRAM_ARB );
}



Bool GPixelShader::Upload()
{
	if ( m_FragmentProgramID != 0 )
	{
		GDebug_Break("Tried to upload pixel shader thats already uploaded\n");
		return FALSE;
	}

	//	enable fragment program operations
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	GDebug::CheckGLError();

	//	create new program entry
	g_DisplayExt.glGenProgramsARB()( 1, &m_FragmentProgramID );

	if ( m_FragmentProgramID == 0 )
	{
		GDebug_Print("Failed to allocate pixel fragment program\n");
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		return FALSE;
	}
	
	//	bind
    g_DisplayExt.glBindProgramARB()( GL_FRAGMENT_PROGRAM_ARB, m_FragmentProgramID );
	GDebug::CheckGLError();

	//	get the fragment program code
	GString FragmentProgram;
	if ( !GetFragmentProgram( FragmentProgram ) || FragmentProgram.Length() == 0 )
	{
		GDebug_Print("Error uploading pixel shader, missing fragment program\n");
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
		Deload();
		return FALSE;
	}
	
	//	load program
	const u8* pProgram = (const u8*)FragmentProgram.Data();
	g_DisplayExt.glProgramStringARB()( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, FragmentProgram.Length(), pProgram );
	//GDebug::CheckGLError();

	//	check for errors
	int ErrorPos = -1;
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &ErrorPos );
	//GDebug::CheckGLError();
	if (ErrorPos != -1 )
	{
		GString Error;
		int bgn = ErrorPos - 10;
		bgn = bgn < 0 ? 0 : bgn;
		char* c = (char*)( pProgram + bgn );
		for(int i = 0; i < 30; i++)
		{
			if(bgn+i >= int(FragmentProgram.Length()-1))
				break;
			Error += *c++;
		}
		GDebug_Print("Some pixel shader loading(syntax) error...\n");
		GDebug_Print( Error.Export() );
		GDebug_Print("\n");

		//	get arb error
		const char* pArbErr = (const char*)glGetString( GL_PROGRAM_ERROR_STRING_ARB );
		GDebug_Break("ARB Error: %s\n", pArbErr );
		
		glDisable(GL_FRAGMENT_PROGRAM_ARB);

		Deload();
		return FALSE;
	}

	
	//	finished uploading
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	GDebug::CheckGLError();

	return TRUE;
}


Bool GPixelShader::Deload()
{
	if ( !HardwareSupported() )
		return FALSE;

	//	already deloaded
	if ( m_FragmentProgramID == 0 )
		return TRUE;

	//	delete program
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	g_DisplayExt.glDeleteProgramsARB()( 1, &m_FragmentProgramID );
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	m_FragmentProgramID = 0;
	GDebug::CheckGLError();

	return TRUE;
}

