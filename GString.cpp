/*------------------------------------------------

  GString.cpp

	String class



-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GString.h"


//	globals
//------------------------------------------------
#define GSTRING_MAX_EXPORT_LEN	5000

//	Definitions
//------------------------------------------------


//-------------------------------------------------------------------------
//	export string into a buffer
//-------------------------------------------------------------------------
void GString::Export(char* pBuffer, int BufferLen) const
{
	memset( pBuffer, 0, BufferLen );

	//	copy as much as we can
	int CopyLen = BufferLen < Length() ? BufferLen : Length();
	
	if ( CopyLen < Length() )
	{
		GDebug_Print("String export capped to %d chars from %d\n", CopyLen, Length() );
	}
	
	memcpy( pBuffer, m_Chars.Data(), CopyLen );
}

//-------------------------------------------------------------------------
//	copy the string into a static buffer and return
//-------------------------------------------------------------------------
char* GString::Export() const
{
	//	multiple buffers so we can use exportstring more than once
	static char BufferA[GSTRING_MAX_EXPORT_LEN];
	static char BufferB[GSTRING_MAX_EXPORT_LEN];
	static char BufferC[GSTRING_MAX_EXPORT_LEN];
	static char BufferD[GSTRING_MAX_EXPORT_LEN];
	static char* g_Buffers[] = {	BufferA,	BufferB,	BufferC,	BufferD	};
	static int ActiveBuffer = 0;
	char* pBuffer = g_Buffers[ (ActiveBuffer++)%ARRAY_SIZE(g_Buffers) ];

	Export( pBuffer, GSTRING_MAX_EXPORT_LEN );

	return pBuffer;
}

//-------------------------------------------------------------------------
//	export string into a buffer
//-------------------------------------------------------------------------
void GString::ExportW(u16* pBuffer, int BufferLen) const
{
	memset( pBuffer, 0, BufferLen );

	//	copy as much as we can
	int CopyLen = BufferLen < Length() ? BufferLen : Length();
	
	if ( CopyLen < Length() )
	{
		GDebug_Print("String exportW capped to %d chars from %d\n", CopyLen, Length() );
	}
	
	for ( int i=0;	i<CopyLen;	i++ )
	{
		pBuffer[i] = (u16)m_Chars[i];
	}
}

//-------------------------------------------------------------------------
//	copy the string into a static buffer and return
//-------------------------------------------------------------------------
u16* GString::ExportW() const
{
	//	multiple buffers so we can use exportstring more than once
	static u16 WBufferA[GSTRING_MAX_EXPORT_LEN];
	static u16 WBufferB[GSTRING_MAX_EXPORT_LEN];
	static u16 WBufferC[GSTRING_MAX_EXPORT_LEN];
	static u16 WBufferD[GSTRING_MAX_EXPORT_LEN];
	static u16* g_WBuffers[] = {	WBufferA,	WBufferB,	WBufferC,	WBufferD	};
	static int ActiveBuffer = 0;
	u16* pWBuffer = g_WBuffers[ (ActiveBuffer++)%ARRAY_SIZE(g_WBuffers) ];

	ExportW( pWBuffer, GSTRING_MAX_EXPORT_LEN );

	return pWBuffer;
}

//-------------------------------------------------------------------------
//	work out a number from our string (integers only)
//-------------------------------------------------------------------------
int GString::GetNumber() const
{
	GList<int> ExtractedNumbers;
	Bool Negative = FALSE;

	for ( int i=0;	i<m_Chars.Size();	i++ )
	{
		const char& c = m_Chars[i];

		//	take out a number
		if ( c >= '0' && c <= '9' )
		{
			int NewDigit = c - '0';
			ExtractedNumbers.Add( NewDigit );
		}

		//	if we come accross a minus, assume this is a minus number
		if ( c == '-' )
			Negative = TRUE;
	}

	//	work backwards through the list, every time we add another, we increase our mutliplier by another factor * 1, then *10, then *100 etc
	int Result = 0;
	int Factor = 1;
	while ( ExtractedNumbers.Size() )
	{
		//	push digit off the end
		int Digit = ExtractedNumbers.ElementLast();
		ExtractedNumbers.RemoveLast();

		Result += Digit * Factor;

		Factor *= 10;
	}

	if ( Negative )
		Result *= -1;

	return Result;
}

//-------------------------------------------------------------------------
//	convert wide chars to regular chars
//-------------------------------------------------------------------------
void GString::Set(const u16* pString)
{
	SetNull();

	if ( pString )
	{
		while ( (*pString) != 0 )
		{
			m_Chars.Add( (char)(*pString) );
			pString++;
		}
	}

	AddTerminator();
}

void GString::Set(const GString& String)
{	
	m_Chars.Copy( String.m_Chars );	
	AddTerminator();	
}

void GString::Set(const GString* pString)					
{
	if ( pString )
		m_Chars.Copy( pString->m_Chars );	
	else
		SetNull();
	
	AddTerminator();	
}

void GString::Set(const char* pString)				
{	
	SetNull();
	Append( pString );	
}

void GString::Set(const char* pString,int Length)		
{	
	SetNull();	
	Append( pString, Length );	
}


//-------------------------------------------------------------------------
//	set as a formatted string
//-------------------------------------------------------------------------
void GString::Setf(const char* pString,...)
{
	char TxtOut[GMAX_STRING];
	if ( strlen( pString ) >= GMAX_STRING )
	{
		GDebug_Break("Formatted string for GString::Setf is too long");
	}

	va_list v;
	va_start( v, pString );
	int iChars = vsprintf((char*)TxtOut,pString, v );
	
	Set( TxtOut );
}

//-------------------------------------------------------------------------
//	append a formatted string
//-------------------------------------------------------------------------
void GString::Appendf(const char* pString,...)
{
	char TxtOut[GMAX_STRING];
	if ( strlen( pString ) >= GMAX_STRING )
	{
		GDebug_Break("Formatted string for GString::Appendf is too long");
	}

	va_list v;
	va_start( v, pString );
	int iChars = vsprintf((char*)TxtOut,pString, v );
	
	Append( TxtOut );
}



//-------------------------------------------------------------------------
//	set the string to a formatted number, eg 12345 becomes 12,345
//-------------------------------------------------------------------------
void GString::SetNumber(int Number,Bool AddCommas)
{
	//	simple number
	if ( !AddCommas )
	{
		Set( ExportStringf("%d",Number) );
		return;
	}

	//	add commas to string
	char ThreeNumbers[4] = { 0,0,0,0 };
	int CurrentNumber = Number;
	int Factor = 1;

	//	clear out current data
	m_Chars.Resize(0);
	
	int loop=0;
	while (TRUE)
	{
		int NumberTen;
		int NumberHundred;
		int NumberThousand;

		if ( CurrentNumber > 0 )
		{
			NumberTen = CurrentNumber;
			NumberTen %= 10;
			m_Chars.Insert(0,'0'+NumberTen);
		}

		if ( CurrentNumber > 9 )
		{
			NumberHundred = ( CurrentNumber - NumberTen ) / 10;
			NumberHundred %= 10;
			m_Chars.Insert(0,'0'+NumberHundred);
		}

		if ( CurrentNumber > 99 )
		{
			NumberThousand = ( CurrentNumber - (NumberHundred*10) - NumberTen ) / 100;
			NumberThousand %= 10;
			m_Chars.Insert(0,'0'+NumberThousand);
		}

		Factor *= 1000;
		CurrentNumber /= Factor;

		if ( CurrentNumber == 0 )
			break;

		m_Chars.Insert(0,',');

	}
}

	
//-------------------------------------------------------------------------
//	check strings match
//-------------------------------------------------------------------------
Bool GString::Equals(const GString& String) const
{
	//	lengths differ, cannot equal!
	if ( String.Length() != Length() )
		return FALSE;

	int Index = 0;
	while ( Index < Length() )
	{
		if ( m_Chars[Index] != String[Index] )
			return FALSE;

		Index++;
	}

	//	no non-matching chars, strings match
	return TRUE;
}


//-------------------------------------------------------------------------
//	check strings match
//-------------------------------------------------------------------------
Bool GString::Equals(const char* pString) const
{
	int Index = 0;
	while ( TRUE )
	{
		//	check lengths
		if ( pString[Index] == 0 )
		{
			//	reached the end of both strings at the same length
			if ( Index == Length() )
			{
				return TRUE;
			}

			//	lengths differ and run out of string
			return FALSE;
		}

		//	run out of our string
		if ( Index == Length() )
			return FALSE;

		//	compare characters
		if ( pString[Index] != m_Chars[Index] )
			return FALSE;

		//	try next char
		Index++;
	}

	//	can't reach here
	return FALSE;
}



void GString::RemoveTerminator()
{
	//	if last character is a terminator, remove it
	int len = Length();
	if ( len != 0 )
	{
		if ( m_Chars[len] == 0 )
		{
			m_Chars.RemoveAt( len );
		}
	}
}

void GString::AddTerminator()
{
	//	add a terminator character at the end if there isnt one
	int len = Length();
	if ( len != 0 )
	{
		if ( m_Chars[len] != 0 )
		{
			m_Chars.Add( (char)0 );
		}
	}
}

