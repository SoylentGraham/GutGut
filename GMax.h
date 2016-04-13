/*------------------------------------------------

  GMax Header file



-------------------------------------------------*/

#if ( !defined(__GMAX__H_) && defined(GUT_MAXSDK_SUPPORT))
#define __GMAX__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"
#include "GMesh.h"
#include "GApp.h"


#define __TRIG__	//	stop the sdk from including the trig.h
#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>

#pragma comment( lib, "acap.lib" )
#pragma comment( lib, "bmm.lib" )
//#pragma comment( lib, "client.lib" )
#pragma comment( lib, "core.lib" )
#pragma comment( lib, "edmodel.lib" )
#pragma comment( lib, "expr.lib" )
#pragma comment( lib, "flt.lib" )
//#pragma comment( lib, "gcomm.lib" )
#pragma comment( lib, "geom.lib" )
#pragma comment( lib, "gfx.lib" )
//#pragma comment( lib, "lvsio.lib" )
#pragma comment( lib, "maxscrpt.lib" )
#pragma comment( lib, "maxutil.lib" )
#pragma comment( lib, "mesh.lib" )
#pragma comment( lib, "mnmath.lib" )
#pragma comment( lib, "paramblk2.lib" )
#pragma comment( lib, "particle.lib" )
//#pragma comment( lib, "patch.lib" )
#pragma comment( lib, "renderutil.lib" )
#pragma comment( lib, "tessint.lib" )
#pragma comment( lib, "viewfile.lib" )

//#pragma comment( lib, "flilibd.Lib" )	//	debug
#pragma comment( lib, "flilibh.lib" )	//	hybrid
//#pragma comment( lib, "flilibr.lib" )	//	release


//	Macros
//------------------------------------------------
#define GUTMAX_BASE_CLASS_ID(n)	Class_ID(0x21a7c8cc, (0x41de9cb2|(n)) )
#define g_MaxExportManager		(GMaxExportManager::g_pManager)





//	Types
//------------------------------------------------
class GMaxMesh;
class GMaxExporter;
class GMaxExportManager;
class GMaxExporterDesc;
class GMaxSceneExporter;
typedef GList<GMaxMesh*>	GMaxMeshList;





/*
	3DSMax mesh. regular mesh with additional mesh info
 */
class GMaxMesh : public GMesh
{
public:
	char				m_Name[255];		//	name in max
	GList<int>			m_Variable;			//	misc variable for use by overloaded exporters

	Bool				m_HasMesh;			//	this node has mesh info
	Matrix3				m_Matrix;			//	transform and rotation matrix from max
	float4				m_Colour;			//	colour out of max

	GList<GMaxMesh*>	m_ChildMeshes;		//	child meshes from this mesh's node
	GMaxMesh*			m_pParentMesh;		//	owner of which this is a child of

public:
	GMaxMesh();
	~GMaxMesh();

	static Bool			CanExport(INode* pNode);		//	is this node exportable?
	Bool				GenerateFromNode(INode* pNode);	//	export this node and add generate any children too
	void				DeleteChildren();
	void				AddChild(GMaxMesh* pMaxMesh);	//	add a child to the list
};



/*
	engine exporter type to overload. added so we only need to overload one class
*/
class GMaxExporter
{
public:

public:
	GMaxExporter()			{	};
	~GMaxExporter()			{	};
	
	virtual const char*		Extension()			{	return "Gut";	};				//	file type extension
	virtual const char*		Description()		{	return "GutGut File";	};		//	file type description
	virtual const char*		Category()			{	return "GutGut";	};			//	category of plugins
	virtual u32				Version()			{	return 100;	};					//	version of exporter

	virtual Bool			Export(const char* pFilename, GMaxMeshList& MeshList)	{	return FALSE;	};
};




typedef struct
{
	GMaxExporter*			pExporter;
	GMaxExporterDesc*		pDesc;

} GMaxExporterEntry;


/*
	exporter manager
*/
class GMaxExportManager
{
	friend GMaxExporterDesc;
	friend GMaxSceneExporter;
	
public:
	static GMaxExportManager*	g_pManager;

protected:
	GList<GMaxMesh*>			m_SceneMeshes;

private:
	GList<GMaxExporterEntry>	m_Exporters;

public:
	GMaxExportManager();
	~GMaxExportManager();
	
	void						ShowAbout();

	void						Add(GMaxExporter* pExporter);
	inline int					ExporterCount()					{	return m_Exporters.Size();	};
	inline GMaxExporter*		Exporter(int Index)				{	return m_Exporters[Index].pExporter;	};
	inline GMaxExporterDesc*	ExporterDesc(int Index)			{	return m_Exporters[Index].pDesc;	};

	void						DeleteMeshes();
	Bool						RecurseNodes(Interface* pInterface);	//	creates list of meshes from the scene

private:
	const char*		AuthorName()			{	return "Graham Reeves";	};
	const char*		CopyrightMessage()		{	return "Copyright(C) 2003 Graham Reeves";	};
	const char*		OtherMessage1()			{	return "Created from the GutGut engine";	};
	const char*		OtherMessage2()			{	return "www.fatgrah.am";	};

	int				GetExporterIndex(GMaxExporterDesc* pDesc);
	GMaxExporter*	GetExporter(GMaxExporterDesc* pDesc)		{	return m_Exporters[ GetExporterIndex(pDesc) ].pExporter;	};
};






/*
	max exporter wrapper. contains info on what exporters are availible. doesnt need to be overloaded
*/
class GMaxSceneExporter : public SceneExport 
{
private:
	GMaxExporterDesc*	m_pDescriptor;
	
public:
	GMaxSceneExporter(GMaxExporterDesc* pDescriptor)	{	m_pDescriptor = pDescriptor;	};
	virtual ~GMaxSceneExporter()						{	};

	GMaxExporter*			Exporter()				{	return g_MaxExportManager->GetExporter(m_pDescriptor);	};

	virtual int				DoExport(const char *pFilename,ExpInterface *pExpInterface,Interface *pInterface, BOOL suppressPrompts=FALSE, DWORD options=0);
	virtual int				ExtCount()				{	return 1;	};
	virtual const char*		Ext(int n)				{	return Exporter()->Extension();	};
	virtual const char*		LongDesc()				{	return Exporter()->Description();	};
	virtual const char*		ShortDesc()				{	return Exporter()->Description();	};
	
	virtual const char*		AuthorName()			{	return g_MaxExportManager->AuthorName();	};
	virtual const char*		CopyrightMessage()		{	return g_MaxExportManager->CopyrightMessage();	};
	virtual const char*		OtherMessage1()			{	return g_MaxExportManager->OtherMessage1();	};
	virtual const char*		OtherMessage2()			{	return g_MaxExportManager->OtherMessage2();	};

	virtual u32				Version()				{	return Exporter()->Version();	};
	virtual void			ShowAbout(HWND hWnd)	{	g_MaxExportManager->ShowAbout();	};
	virtual BOOL			SupportsOptions(int ext, DWORD options)	{	return TRUE;	};		// Returns TRUE if all option bits set are supported for this extension
};




/*
	internal gutmax exporter description. most functions call back to the exporter manager to send to the overloaded GMaxExporter type
*/
class GMaxExporterDesc : public ClassDesc2 
{
public:
	GMaxExporter*	Exporter()					{	return g_MaxExportManager->GetExporter(this);	};
	int				ExporterIndex()				{	return g_MaxExportManager->GetExporterIndex(this);	};

	int 			IsPublic()					{	return TRUE;	}
	void *			Create(BOOL loading=FALSE)	{	return new GMaxSceneExporter(this);	}
	const TCHAR *	ClassName()					{	return Exporter()->Description(); }
	SClass_ID		SuperClassID()				{	return SCENE_EXPORT_CLASS_ID;	}
	Class_ID		ClassID()					{	return GUTMAX_BASE_CLASS_ID(ExporterIndex());	}
	const TCHAR* 	Category()					{	return Exporter()->Category();	}
	const TCHAR*	InternalName()				{	return Exporter()->Description();	}
	HINSTANCE		HInstance()					{	return GApp::g_HInstance;	}
};







//	Declarations
//------------------------------------------------
DLL_EXPORT ClassDesc*	LibClassDesc(int i);
DLL_EXPORT int			LibNumberClasses();
DLL_EXPORT ULONG		LibVersion();
DLL_EXPORT const char*	LibDescription();
void					CopyMatrix( GMatrix& DestMatrix, Matrix3& SourceMatrix );

BOOL WINAPI		DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved);



//	Inline Definitions
//-------------------------------------------------


#endif

