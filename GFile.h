/*------------------------------------------------

  GFile Header file



-------------------------------------------------*/

#ifndef __GFILE__H_
#define __GFILE__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GBinaryData.h"


//	Macros
//------------------------------------------------





//	Types
//------------------------------------------------
class GString;

//--------------------------------------------------------------------------------------------------------
// simple file access
//--------------------------------------------------------------------------------------------------------
class GFile
{
public:
	GBinaryData		m_Data;		//	raw data loaded, or to be saved

public:
	GFile();
	~GFile();

	Bool	Load(const GString& Filename);	//	load all the data out of the specified filename and into this class
	Bool	Save(const GString& Filename);	//	save all the data out of this and into a file
	
};




//	Declarations
//------------------------------------------------
Bool			ExtractFilePath(const GString& InPath, GString& OutPath);	//	find the path for this file eg. c:/foo/bar.x -> c:/foo/
Bool			ExtractFullFilePath(const GString& InPath, GString& OutPath);	//	find the full path for this file eg. c:/foo/bar.x -> c:/foo/, but NOT ../foo/bar.x 




//	Inline Definitions
//-------------------------------------------------


#endif

