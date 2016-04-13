#ifdef GUT_MAXSDK_SUPPORT
/*------------------------------------------------

  GMax.cpp

	MaxSDK functions
	not compiled if GUT_MAXSDK_SUPPORT is not defined


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GMax.h"
#include "GAssetList.h"


//	globals
//------------------------------------------------
GMaxExportManager*	GMaxExportManager::g_pManager = NULL;



//	Definitions
//------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	//	need to check the DLLInit so we dont initialise more than once
	if ( !g_DLLInit )
	{
		//	set instance
		GApp::g_HInstance = hinstDLL;

		//	intialise win32 and debug stuff
		if ( ! GWin32::Init() )	return -1;

		//	mok-a-moka-mok
		if ( GetSystemMetrics(SM_SLOWMACHINE) != 0 )
		{
			GDebug::Print("In soviet russia, your HARDWARE says WINDOWS is slow\n");
		}

		//	initialise app
		if ( !g_App )
		{
			GDebug_Break("No app declared");
			return -1;
		}
		if ( !g_App->DllInit() )
		{
			GDebug_Break("App failed to DllInit");
			return -1;
		}

		g_App->SetPath( "" );

		g_DLLInit = TRUE;
	}

	return TRUE;
}



DLL_EXPORT ClassDesc* LibClassDesc(int i)
{
	if ( !g_MaxExportManager )
	{
		GDebug_Break("No exporter for max plugin\n");
		return NULL;
	}

	if ( i >= g_MaxExportManager->ExporterCount() )
	{
		return NULL;
	}

	return (ClassDesc2*)g_MaxExportManager->ExporterDesc(i);
}

DLL_EXPORT int LibNumberClasses()
{
	if ( !g_MaxExportManager )
	{
		GDebug_Break("No exporter for max plugin\n");
		return 0;
	}

	return g_MaxExportManager->ExporterCount();
}

DLL_EXPORT ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

DLL_EXPORT const char* LibDescription()
{
	if ( ! g_App )
	{
		GDebug_Break("No app for max plugin\n");
		return "Description";
	}
	
	return g_App->GameDescription();
}



//------------------------------------------------



GMaxMesh::GMaxMesh()
{
	//	init vars
	strcpy(m_Name, "Unknown");
	m_pParentMesh = NULL;

	m_HasMesh	= FALSE;
//	m_Matrix	= Matrix3::Identity;	//	missing from maxsdk. fs.
	m_Colour	= float4(1,1,1,1);

}


GMaxMesh::~GMaxMesh()
{
	//	delete child meshes
	DeleteChildren();

	//	remove from global mesh list if its in there before we finish deleting it
	GAssets::g_Meshes.Remove( this );
}
	

void GMaxMesh::DeleteChildren()
{
	//	delete meshes then empty list
	for ( int i=0;	i<m_ChildMeshes.Size();	i++ )
	{
		GDelete( m_ChildMeshes[i] );
	}
	m_ChildMeshes.Empty();
}

	
void GMaxMesh::AddChild(GMaxMesh* pMaxMesh)
{
	m_ChildMeshes.Add( pMaxMesh );
}


/*static*/Bool GMaxMesh::CanExport(INode* pNode)
{
	return TRUE;
}


Bool GMaxMesh::GenerateFromNode(INode* pNode)
{
	//	check we can get data out of this node
	int Time = 0;
	Object* pObject = pNode->EvalWorldState(Time).obj;

	//	node doesnt have an object
	if ( !pObject )
		return FALSE;

	//	check it can be converted to a triangle mesh
	m_HasMesh = pObject->CanConvertToType( triObjectClassID );

	Bool DeleteObj = FALSE;

	//	generate GutMesh from max triangle mesh
	m_Matrix = pNode->GetObjectTM( Time );

	//	setup mesh info
	strcpy( m_Name, pNode->GetName() );
	
	//	convert colour of mesh to float4
	u16 RGBColour = pNode->GetWireColor();
	m_Colour[0] = RGB16_Redf( RGBColour );
	m_Colour[1] = RGB16_Greenf( RGBColour );
	m_Colour[2] = RGB16_Bluef( RGBColour );
	m_Colour[3] = 1.f;	//	alpha
	
	int r = RGB16_Red( RGBColour );
	int g = RGB16_Green( RGBColour );
	int b = RGB16_Blue( RGBColour );

	if ( m_HasMesh )
	{
		TriObject* pMeshObject = NULL;
		
		//	convert the object to a triangle mesh
		pMeshObject = (TriObject*)pObject->ConvertToType( Time, triObjectClassID );
		DeleteObj = (pMeshObject != pObject);

		Mesh& mesh = pMeshObject->GetMesh();
		
		mesh.buildNormals();

		//	debug translation
		Point3 Translation = m_Matrix.GetTrans();
		float3 Pos = float3( Translation.x, Translation.y, Translation.z );
		GDebug::Print("x %3.3f y %3.3f z %3.3f\n\n",Pos.x, Pos.y, Pos.z );

		//	get number of verts
		int NormalVertCount = mesh.getNumVerts();	//	regular vert count
		int TVertCount = mesh.getNumTVerts();		//	texture vert count (different if texture uv's differ on some faces for the same verts)
		if ( TVertCount == 0 )
			TVertCount = NormalVertCount;
	
		GDebug::Print("object \"%s\" has %d faces, %d verts(%d texture verts)\n", m_Name, mesh.getNumFaces(), NormalVertCount, TVertCount );
		
		if ( !mesh.tVerts )
		{
			GDebug::Print("Warning, Mesh \"%s\" has no texture UV information\n",m_Name);
		}

		//	alloc vertexes
		AllocVerts( TVertCount );
		m_Normals.Resize( VertCount() );		//	allocate vertex normals

		if ( mesh.tVerts )
			m_TextureUV.Resize( VertCount() );	//	allocate vertex texture uv's

		//	allocate triangles
		AllocTriangles( mesh.getNumFaces() );


		//	setup verts
		int i;
		for ( i=0;	i<VertCount();	i++ )
		{
			if ( i >= VertCount() )
				break;
			
			Matrix3 ScaleRotMatrix = m_Matrix;
			ScaleRotMatrix.NoTrans();

			Matrix3 RotMatrix = m_Matrix;
			RotMatrix.NoTrans();
			RotMatrix.NoScale();

			//	is a TVert vert, dont get the vert data
			if ( i < NormalVertCount )
			{
				Point3 origv = mesh.verts[i];
				GDebug::CheckFloat( origv.x );
				GDebug::CheckFloat( origv.y );
				GDebug::CheckFloat( origv.z );
				
				Point3 v = origv * ScaleRotMatrix;
				GDebug::CheckFloat( v.x );
				GDebug::CheckFloat( v.y );
				GDebug::CheckFloat( v.z );

				m_Verts[i] = float3( v.x, v.y, v.z );

				GDebug::CheckFloat( m_Verts[i].x );
				GDebug::CheckFloat( m_Verts[i].y );
				GDebug::CheckFloat( m_Verts[i].z );

				Point3 n = mesh.getNormal(i) * RotMatrix;
				m_Normals[i] = float3( n.x, n.y, n.z );
				m_Normals[i].Normalise();

				//	set texture UV
				if ( m_TextureUV.Size() )
				{
					m_TextureUV[i] = float2( 0.f, 0.f );
					if ( mesh.tVerts )
					{
						UVVert& uv = mesh.getTVert(i);
						m_TextureUV[i] = float2( uv.x, uv.y );
					}
				}
			}
			else
			{
				//	initialise unset vert data
				m_Verts[i] = float3( 444.555f, 666.777f, 888.999f );
				m_Normals[i] = float3( 111.222f, 333.444f, 555.666f );
				if ( m_TextureUV.Size() )
				{
					m_TextureUV[i] = float2( 777.888f, 999.111f );
				}
			}

		}


		//	setup triangles
		for ( i=0;	i<TriCount();	i++ )
		{
			//	vertex faces
			Face face = mesh.faces[i];

			//	no extra UV data
			if ( TVertCount == NormalVertCount )
			{
				//	make triangle
				m_Triangles[i] = int3( face.v[0], face.v[1], face.v[2] );
			}
			else
			{
				//	check if we need additional verts for the different UV verts
				TVFace tvFace = mesh.tvFace[i];

				//	orig vert
				int3 origverts( face.v[0], face.v[1], face.v[2] );

				//	triangle vert
				int3 triverts( tvFace.t[0], tvFace.t[1], tvFace.t[2] );

				//	is the UV different?
				for ( int j=0;	j<3;	j++ )
				{
					UVVert tvert_uv = mesh.getTVert( triverts[j] );
					float2 uv( tvert_uv.x, tvert_uv.y );
					if ( uv == m_TextureUV[ origverts[j] ] )
					{
						//	same uv coords, re-use vert
						triverts[j] = origverts[j];
					}
					else
					{
						//	create new vert with different UV coords
						CopyVert( origverts[j], triverts[j] );

						m_TextureUV[ triverts[j] ] = uv;
					}
				}

				//	make triangle
				m_Triangles[i] = triverts;
			}
			
		}


		//	delete the generated mesh, if it was generated
		if ( DeleteObj && pMeshObject )
		{
			delete pMeshObject;
			pMeshObject = NULL;
		}
	}

	//	check in case anything went wrong
	CheckFloats();

	//	remove unused verts and merge
	MergeVerts();


	//	export children
	for ( int c=0;	c<pNode->NumberOfChildren();	c++ )
	{
		INode* pChildNode = pNode->GetChildNode(c);

		if ( GMaxMesh::CanExport( pChildNode ) )
		{
			GMaxMesh* pMesh = new GMaxMesh;
			pMesh->GenerateFromNode( pChildNode );
			pMesh->m_pParentMesh = this;
			AddChild( pMesh );
		}
	}

	return TRUE;
}



//------------------------------------------------


GMaxExportManager::GMaxExportManager()
{
	//	check global pointer first
	if ( GMaxExportManager::g_pManager )
	{
		GDebug_Break("Exporter already created. Can only have one instance of an Exporter\n");
	}

	//	set global pointer
	GMaxExportManager::g_pManager = this;

	//	init vars
}




GMaxExportManager::~GMaxExportManager()
{
	//	cleanup
	if ( GMaxExportManager::g_pManager == this )
	{
		GMaxExportManager::g_pManager = NULL;
	}

	//	delete exporters
	for ( int i=0;	i<m_Exporters.Size();	i++ )
	{
		//	delete exporter
		GDelete( m_Exporters[i].pExporter );

		//	delete descriptor
		GDelete( m_Exporters[i].pDesc );
	}
	m_Exporters.Empty();


	//	delete meshes
	DeleteMeshes();
}







void GMaxExportManager::Add(GMaxExporter* pExporter)
{
	//	create a descriptor then add
	GMaxExporterEntry ExporterEntry;
	ExporterEntry.pExporter = pExporter;
	ExporterEntry.pDesc = new GMaxExporterDesc;
	m_Exporters.Add( ExporterEntry );
}


void GMaxExportManager::DeleteMeshes()
{
	//	delete meshes
	for ( int i=0;	i<m_SceneMeshes.Size();	i++ )
	{
		GDelete( m_SceneMeshes[i] );
	}
	m_SceneMeshes.Empty();
}


Bool GMaxExportManager::RecurseNodes(Interface* pInterface)
{
	//	delete existing mesh list
	DeleteMeshes();

	//	recurse the nodes from the root node into the meshfile
	INode* pRootNode = pInterface->GetRootNode();
	if ( !pRootNode )
	{
		GDebug_Break("Missing root node");
		return FALSE;
	}

	//	add each child
	for ( int c=0;	c<pRootNode->NumberOfChildren();	c++ )
	{
		INode* pChildNode = pRootNode->GetChildNode(c);

		if ( GMaxMesh::CanExport( pChildNode ) )
		{
			GMaxMesh* pMesh = new GMaxMesh;
			pMesh->GenerateFromNode( pChildNode );
			m_SceneMeshes.Add( pMesh );
		}
	}

	return TRUE;
}


int GMaxExportManager::GetExporterIndex(GMaxExporterDesc* pDesc)
{
	//	find matching descriptor
	for ( int i=0;	i<m_Exporters.Size();	i++ )
	{
		if ( m_Exporters[i].pDesc == pDesc )
			return i;
	}

	//	not found
	return -1;
}

void GMaxExportManager::ShowAbout()
{
	GWin32::Popupf( Popup_OK, "About", "GutGut max exporter\n\n%s\n%s\n%s", CopyrightMessage(), OtherMessage1(), OtherMessage2() );
}




//------------------------------------------------



int GMaxSceneExporter::DoExport(const char *pFilename,ExpInterface *pExpInterface,Interface *pInterface, BOOL suppressPrompts, DWORD options)
{
	#define EXPORT_DEBUG	TRUE

	if ( EXPORT_DEBUG )
	{
		GDebug::Show();
	}
	
	//	get meshes
	if ( ! g_MaxExportManager->RecurseNodes(pInterface) )
	{
		GDebug::Print("Failed to recurse nodes\n");
		g_MaxExportManager->DeleteMeshes();

		return FALSE;
	}

	Bool Result = Exporter()->Export( pFilename, g_MaxExportManager->m_SceneMeshes );

	//	cleanup assets
	g_MaxExportManager->DeleteMeshes();
	GAssets::ClearAssets();

	if ( EXPORT_DEBUG )
	{
		//GDebug::Popup("Remove debug info...");
		GDebug::Hide();
	}


	return Result;
}











#endif	//	GUT_MAXSDK_SUPPORT
