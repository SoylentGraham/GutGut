/*------------------------------------------------

  GBinaryData Header file



-------------------------------------------------*/

#ifndef __GBINARYDATA__H_
#define __GBINARYDATA__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"


//	Macros
//------------------------------------------------





//	Types
//------------------------------------------------

//--------------------------------------------------------------------------------------------------------
//	simple-interface class for reading/writing data, mainly used for parsing like a file
//	could be adapted for streaming data in/out
//--------------------------------------------------------------------------------------------------------
class GBinaryData
{
public:
	GList<u8>	m_Data;		//	all the data
	
private:
	int			m_ReadPos;	//	current read pos (bytes)

public:
	GBinaryData();
	~GBinaryData();
	
	Bool			Read(void* pData,int DataSize,const char* pErrTypeString=NULL);		//	read an amount of data from our binary data starting from readpos and then moves the readpos foward
	Bool			Skip(int DataSize);											//	skip over data (as if we'd read it)
	int				Write(void* pData,int DataSize);							//	append unspecified type of data to current data. returns new length
	int				Write(GBinaryData& Data);									//	append binary data to this binary data. returns new length
	inline void		Empty()			{	m_Data.Empty();	};						//	empty out our stored data
	inline void		ResetRead()		{	m_ReadPos = 0;	};						//	move read pos back to start of data

	GList<u8>&		Data()			{	return m_Data;	};
	inline int		DataRead()		{	return m_ReadPos;	};					//	amount of data that has been read
	inline int		DataUnread()	{	return m_Data.DataSize() - m_ReadPos;	};	//	remaining amount of data not yet read
	inline int		Size()			{	return m_Data.DataSize();	};				//	total amount of data
	inline int		GetReadPos()	{	return m_ReadPos;	};
	Bool			SetReadPos(int NewReadPos);
};



//	Declarations
//------------------------------------------------



//	Inline Definitions
//-------------------------------------------------


#endif

