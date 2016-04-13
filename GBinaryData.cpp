/*------------------------------------------------

  GBinaryData.cpp

  Class for easy parsing of data with debug checks etc

-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GGutFile.h"


//	globals
//------------------------------------------------


//	Definitions
//------------------------------------------------


GBinaryData::GBinaryData()
{
	m_ReadPos = 0;
}

GBinaryData::~GBinaryData()
{
	Empty();
}

//--------------------------------------------------------------------------------------------------------
//	read an amount of data from our binary data starting from readpos and then moves the readpos foward
//--------------------------------------------------------------------------------------------------------
Bool GBinaryData::Read(void* pData,int DataSize,const char* pErrTypeString)
{
	//	check params
	if ( DataSize == 0 )
	{
		GDebug_Print("Warning: Tried to read 0 bytes of data from GBinaryData %s\n", pErrTypeString );
		return TRUE;
	}

	if ( !pData )
	{
		GDebug_Break("Warning: Tried to read data(%d bytes) into NULL address. %s\n", DataSize, pErrTypeString );
		return FALSE;
	}

	//	check if we've run out of data
	if ( DataSize > DataUnread() )
	{
		GString ErrorString;
		ErrorString += "Error reading binary data ";
		if ( pErrTypeString != NULL )
		{
			ErrorString.Appendf("\"%s\"; ", pErrTypeString );
		}
		ErrorString.Appendf("%d bytes, only %d bytes remaining", DataSize, DataUnread() );
		GDebug_Break( ErrorString );
		
		return FALSE;
	}

	//	copy data
	memcpy( pData, &m_Data[m_ReadPos], DataSize );

	//	move read marker
	m_ReadPos += DataSize;

	return TRUE;
}


//--------------------------------------------------------------------------------------------------------
//	read an amount of data from our binary data starting from readpos and then moves the readpos foward
//--------------------------------------------------------------------------------------------------------
Bool GBinaryData::Skip(int DataSize)
{
	//	check params
	if ( DataSize == 0 )
	{
		GDebug_Print("Warning: Tried to read 0 bytes of data from GBinaryData\n" );
		return TRUE;
	}

	//	check if we've run out of data
	if ( DataSize > DataUnread() )
	{
		GString ErrorString;
		ErrorString += "Error skipping binary data ";
		ErrorString.Appendf("%d bytes, only %d bytes remaining", DataSize, DataUnread() );
		GDebug_Break( ErrorString );
		
		return FALSE;
	}

	//	move read marker
	m_ReadPos += DataSize;

	return TRUE;
}

//--------------------------------------------------------------------------------------------------------
//	set a new read pos, but check position etc
//--------------------------------------------------------------------------------------------------------
Bool GBinaryData::SetReadPos(int NewReadPos)
{
	//	negative
	if ( NewReadPos < 0 )
	{
		GDebug_Break("Tried to set read position to an invalid position %d", NewReadPos );
		return FALSE;
	}

	//	check in limits
	//	check if we've run out of data
	if ( NewReadPos > Size() )
	{
		GString ErrorString;
		ErrorString.Appendf("Error setting binary data read pos to %d out of %d", NewReadPos, Size() );
		GDebug_Break( ErrorString );
		return FALSE;
	}

	m_ReadPos = NewReadPos;

	return TRUE;
}


//--------------------------------------------------------------------------------------------------------
//	append unspecified type of data to current data. returns new length
//--------------------------------------------------------------------------------------------------------
int GBinaryData::Write(void* pData,int DataSize)
{
	//	append data to our data
	m_Data.Add( (u8*)pData, DataSize );
	
	return Size();
}

//--------------------------------------------------------------------------------------------------------
//	append binary data to this binary data. returns new length
//--------------------------------------------------------------------------------------------------------
int GBinaryData::Write(GBinaryData& Data)
{
	//	append data to our data
	m_Data.Add( Data.m_Data );
	
	return Size();
}



