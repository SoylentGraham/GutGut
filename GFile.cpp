/*------------------------------------------------

  GFile.cpp

	Class for reading and writing data to a file



-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GFile.h"
#include "GDebug.h"
#include "GWin32.h"
#include "GString.h"


//	globals
//------------------------------------------------


//	Definitions
//------------------------------------------------



GFile::GFile()
{
}



GFile::~GFile()
{
	//	delete all data
	m_Data.Empty();
}



Bool GFile::Load(const GString& Filename)
{
	//	empty current data
	m_Data.Empty();
	
	GString LoadFilename( Filename );

	//	if filename doesnt have a path, insert our global path
	if ( !ExtractFullFilePath( Filename, LoadFilename ) )
	{
		LoadFilename.Insert( 0, GApp::g_AppPath );
	}

	//	open a file for read in binary
	FILE* File = fopen( LoadFilename, "rb" );
	if ( !File )
	{
		GDebug_Print("Error opening file \"%s\"\n", (char*)LoadFilename );
		return FALSE;
	}
	
	//	how big is the file? (goto the end, and see where the file pos is)
	int FileSize = 0;
	fseek( File, 0, SEEK_END );
	FileSize = ftell(File);

	//	alloc data for file
	m_Data.m_Data.Resize( FileSize );

	//	read data
	fseek( File, 0, SEEK_SET );
	fread( m_Data.Data().Data(), m_Data.Size(), 1, File );
	fclose( File );

	return TRUE;
}




Bool GFile::Save(const GString& Filename)
{
	//	no data?
	if ( m_Data.Size() == 0 )
	{
		GDebug_Break("No data to save\n");
		return FALSE;
	}

	GString SaveFilename( Filename );

	//	if filename doesnt have a path, insert our global path
	if ( !ExtractFilePath( Filename, SaveFilename ) )
	{
		SaveFilename.Insert( 0, GApp::g_AppPath );
	}

	//	open a file for write in binary
	FILE* File = fopen( SaveFilename, "wb" );
	if ( !File )
	{
		GDebug_Print("Error creating file \"%s\".", (char*)SaveFilename );
		return FALSE;
	}
	
	//	write data
	fwrite( m_Data.Data().Data(), m_Data.Size(), 1, File );
	fclose( File );
	
	return TRUE;
}





//-------------------------------------------------------------------------
//	find the path for this file eg. c:\foo\bar.x -> c:\foo\
//-------------------------------------------------------------------------
Bool ExtractFilePath(const GString& InPath, GString& OutPath)
{
	OutPath = InPath;

	//	go back through the string until we reach a \ or /
	int i;
	for ( i=OutPath.Length();	i>=0;	i-- )
	{
		char& c = OutPath[i];

		//	found a slash
		if ( c == '\\' || c == '/' )
			break;

		//	not a slash, remove the char
		OutPath.Chars().RemoveAt(i);
	}

	//	didnt reach a slash, failed
	if ( i <= 0 )
	{
		return FALSE;
	}

	//	return string
	return TRUE;
}

//-------------------------------------------------------------------------
//	find the full path for this file eg. c:\foo\, but NOT ..\foo\
//-------------------------------------------------------------------------
Bool ExtractFullFilePath(const GString& InPath, GString& OutPath)
{
	//	find a ":/" or ":\" near the start and then do the usual extract
	int i;
	int len = InPath.Length();
	Bool LastColon = FALSE;

	for ( i=0;	i<len;	i++ )
	{
		const char& c = InPath[i];

		//	found colon last time, now find the slash after it
		if ( LastColon )
		{
			if ( c == '\\' || c == '/' )
			{
				return ExtractFilePath( InPath, OutPath );
				break;
			}

			LastColon = FALSE;
		}

		//	is this a colon?
		if ( c == ':' )
			LastColon = TRUE;
	}

	//	failed to match
	return FALSE;
}
