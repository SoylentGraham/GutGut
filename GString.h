/*------------------------------------------------

  GString Header file



-------------------------------------------------*/

#ifndef __GSTRING__H_
#define __GSTRING__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GList.h"



//	Macros
//------------------------------------------------




//	Types
//------------------------------------------------

class GString
{
private:
	GList<char>			m_Chars;

public:
	GString()									{	};
	GString(const char* pString)				{	Set( pString );	};
	GString(const char* pString, int Length)	{	Set( pString, Length );	};
	GString(const u16* pWString)				{	Set( pWString );	};
	GString(const GString& String)				{	Set( String );	};
	GString(const GString* pString)				{	Set( pString );	};
	GString(int Number,Bool AddCommas=FALSE)	{	SetNumber(Number,AddCommas);	};
	~GString()									{	};

	const char*			DataConst() const							{	return m_Chars.DataConst();	};
	char*				Data() const								{	return m_Chars.Data();	};
	void				Export(char* pBuffer, int BufferLen) const;	//	export string into a buffer
	char*				Export() const;								//	export string
	void				ExportW(u16* pBuffer, int BufferLen) const;	//	export widechar string into a buffer
	u16*				ExportW() const;							//	export widechar string
	int					GetNumber() const;							//	works out a number from the string
	void				SetNumber(int Number,Bool AddCommas=FALSE);	//	converts a number to a string, addcomma's formats the string like 1,024 instead of 1024
	int					Length() const								{	return m_Chars.Size() ? m_Chars.Size()-1 : 0;	};	//	returns length of string, -1 to ignore terminator character
	void				Clear()										{	m_Chars.Empty();	};
	inline GList<char>&	Chars()										{	return m_Chars;	};
	
	void				Set(const GString& String);
	void				Set(const GString* pString);
	void				Set(const char* pString);
	void				Set(const char* pString,int Length);
	void				Set(const u16* pString);				//	convert widechars to string and copy
	void				Setf(const char* pString,...);
	void				SetNull()								{	m_Chars.Resize(0);	};	//	clear string

	void				Append(const GString& String)			{	RemoveTerminator();	m_Chars.Add( String.m_Chars );	AddTerminator();	};
	void				Append(const GString* pString)			{	RemoveTerminator();	m_Chars.Add( pString->m_Chars );	AddTerminator();	};
	void				Append(const char* pString)				{	RemoveTerminator();	m_Chars.Add( (char*)pString, strlen( pString ) );	AddTerminator();	};
	void				Append(const char* pString, int Length)	{	RemoveTerminator();	m_Chars.Add( (char*)pString, Length );	AddTerminator();	};
	void				Appendf(const char* pString,...);
	void				Append(int Number,Bool AddCommas=FALSE)	{	Append( GString(Number,AddCommas) );	};

	void				Insert(int Position,GString& String)	{	RemoveTerminator();	String.RemoveTerminator();	m_Chars.Insert( Position, String.m_Chars );	AddTerminator();	String.AddTerminator();	};
	void				Insert(int Position,GString* pString)	{	RemoveTerminator();	pString->RemoveTerminator();	m_Chars.Insert( Position, pString->m_Chars );	AddTerminator();	pString->AddTerminator();	};
	void				Insert(int Position,const char* pString){	RemoveTerminator();	m_Chars.Insert( Position, (char*)pString, strlen( pString ) );	AddTerminator();	};

	Bool				Equals(const GString& String) const;
	Bool				Equals(const char* pString) const;

	inline char&		operator[](int Index)					{	return m_Chars[Index];	}
	inline char&		operator[](u32 Index)					{	return m_Chars[Index];	}
	inline const char&	operator[](int Index) const				{	return m_Chars[Index];	}
	inline const char&	operator[](u32 Index) const				{	return m_Chars[Index];	}
	
	inline void			operator=(const char* pString)			{	Set( pString );	};
	inline void			operator=(const GString& String)		{	Set( String );	};
	inline void			operator=(const GString* pString)		{	Set( pString );	};

	inline void			operator+=(const char* pString)			{	Append(pString);	};
	inline void			operator+=(const GString& String)		{	Append(String);	};
	inline void			operator+=(const GString* pString)		{	Append(pString);	};
	inline void			operator+=(const char Character)		{	m_Chars.Add( Character );	};

	inline Bool			operator==(const GString& String) const	{	return Equals( String );	};
	inline Bool			operator==(const char* pString) const	{	return Equals( pString );	};
	inline Bool			operator!=(const GString& String) const	{	return !Equals( String );	};
	inline Bool			operator!=(const char* pString) const	{	return !Equals( pString );	};

	inline				operator char*()						{	return Data();	};
	inline				operator const char*() const			{	return DataConst();	};
	
private:
	void				RemoveTerminator();
	void				AddTerminator();
};



//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------

//-------------------------------------------------------------------------
//	special copy class for strings
//-------------------------------------------------------------------------
inline void GCopyData(GString* pNewData, const GString* pOldData, int Elements)
{
	//	manually copy each string
	for ( int i=0;	i<Elements;	i++ )
	{
		pNewData[i].Set( pOldData[i] );
	}
}


#endif

