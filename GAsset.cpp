/*------------------------------------------------

  GAsset.cpp

	Base asset type for data.


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GAsset.h"



//	globals
//------------------------------------------------
const char* g_AssetTypeNames[ GAssetMax ] = 
{
	"Unknown",
	"Mesh",
	"Texture",
	"Object",
	"Map",
	"SubMap",
	"MapObject",
	"Skeleton",
	"Skin",
	"SkeletonAnim",
	"Font",
};


//	table for string->ref conversions
#define		AssetRefCharTableSizeMax	64	//	
#define		AssetRefCharTableSize		41	//	supported chars in a ref
#define		AssetCharIndexBitMask	(BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5))

const char	g_AssetRefCharTable[AssetRefCharTableSize+1]	= {	"?abcdefghijklmnopqrstuvwxyz0123456789 -#_"	};
const char	g_AssetRefCharTableAlt[AssetRefCharTableSize+1] = {	"?ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -#_"	};

//	Definitions
//------------------------------------------------




GAsset::GAsset()
{
	m_AssetRef = GAssetRef_Invalid;
}




GAsset::~GAsset()
{
}



Bool GAsset::LoadAsset(GAssetHeader* pAssetHeader,GBinaryData& Data)
{
	//	check version
	if ( pAssetHeader->AssetVersion != Version() )
	{
		GDebug::Print("%s version mismatch: 0x%08x should be 0x%08x\n", AssetTypeName(), pAssetHeader->AssetVersion, Version() );
		return FALSE;
	}

	//	copy generic asset data from asset header
	m_AssetRef = pAssetHeader->AssetRef;

	return Load( Data );
}


Bool GAsset::Load(GBinaryData& Data)
{
	return FALSE;
}




Bool GAsset::Save(GBinaryData& Data)
{
	return FALSE;
}



u32 GetAssetRefCharIndex(char c)
{
	for ( u32 i=0;	i<AssetRefCharTableSize;	i++ )
	{
		if ( g_AssetRefCharTable[i] == c )
			return i;

		if ( g_AssetRefCharTableAlt[i] == c )
			return i;
	}

	//	unsupported char == ?
	return GetAssetRefCharIndex( '?' );
}


GAssetRef GAsset::NameToRef(const char* String)
{
	int i;
	u32 Ref = 0;

	//	make a table of ref char indexes
	u32 CharIndexes[AssetRefCharCount];
	for ( i=0;	i<AssetRefCharCount;	i++ )
	{
		if ( String[0] == 0 )
			CharIndexes[i] = GetAssetRefCharIndex('?');
		else
			CharIndexes[i] = GetAssetRefCharIndex(' ');
	}

	//	go through the string
	for ( i=0;	i<AssetRefCharCount;	i++ )
	{
		if ( String[i] == 0 )
			break;
		CharIndexes[i] = GetAssetRefCharIndex(String[i]);
	}

	//	create u32 from char numbers
	int BitIndex = 0;
	
	//	do this for i<AssetRefCharCount
	Ref |= ( CharIndexes[0] & AssetCharIndexBitMask ) << (0*6);
	Ref |= ( CharIndexes[1] & AssetCharIndexBitMask ) << (1*6);
	Ref |= ( CharIndexes[2] & AssetCharIndexBitMask ) << (2*6);
	Ref |= ( CharIndexes[3] & AssetCharIndexBitMask ) << (3*6);
	Ref |= ( CharIndexes[4] & AssetCharIndexBitMask ) << (4*6);

	return Ref;
}


char* GAsset::RefToName(GAssetRef Ref)
{
	char Buffer[AssetRefCharCount+1];
	
	//	gather indexes
	u32 CharIndexes[AssetRefCharCount];

	int i;
	int BitIndex = 0;
	
	for ( i=0;	i<AssetRefCharCount;	i++ )
	{
		CharIndexes[i] = ( Ref >> BitIndex ) & AssetCharIndexBitMask;
		BitIndex += 6;
	}

	//	convert indexes to string
	for ( i=0;	i<AssetRefCharCount;	i++ )
	{
		Buffer[i] = g_AssetRefCharTable[ CharIndexes[i] ];
	}

	//	add string terminator
	Buffer[AssetRefCharCount] = 0;

	return ExportString( Buffer );
}



GAssetRef IncrementAssetRef(GAssetRef& AssetRef)
{
	u32 OldRef = AssetRef;

	//	gather indexes
	u32 CharIndexes[AssetRefCharCount];

	int i;
	int BitIndex = 0;

	for ( i=0;	i<AssetRefCharCount;	i++ )
	{
		CharIndexes[i] = ( AssetRef >> BitIndex ) & AssetCharIndexBitMask;
		BitIndex += 6;
	}

	Bool Changed = FALSE;
	
	/*
	//	if theres a symbol on the end, turn it into a number
	if ( !Changed )
	{
		u32 AIndex = GetAssetRefCharIndex('a');
		u32 ZIndex = GetAssetRefCharIndex('z');
		u32 ZeroIndex = GetAssetRefCharIndex('0');
		u32 NineIndex = GetAssetRefCharIndex('9');

		i = AssetRefCharCount-1;
		{
			//	if not a letter
			if ( CharIndexes[i] < AIndex || CharIndexes[i] > ZIndex )
			{
				//	not a number
				if ( CharIndexes[i] < ZeroIndex || CharIndexes[i] > NineIndex )
				{
					CharIndexes[i] = GetAssetRefCharIndex('0');
					Changed = TRUE;
				}
				else
				{
					//	a number 0..8
					if ( CharIndexes[i] != NineIndex )
					{
						CharIndexes[i]++;
						Changed = TRUE;
					}
					else
					{
						//	nine, try to see if we can convert say 39 to 40
						if ( CharIndexes[i-1] >= ZeroIndex && CharIndexes[i-1] < NineIndex )
						{
							CharIndexes[i-1]++;
							CharIndexes[i] = ZeroIndex;
							Changed = TRUE;
						}
						else if ( CharIndexes[i-1] == NineIndex )
						{
							//	previous number is nine, reset 99 to 00
							CharIndexes[i-1] = ZeroIndex;
							CharIndexes[i] = ZeroIndex;
							Changed = TRUE;
						}
						else
						{
							//	previous char not a number
						}
					}
				}
			}
		}
	}


	
	//	still not changed, increment the character on the end
	if ( !Changed )
	{
		i = AssetRefCharCount-1;
		
		if ( CharIndexes[i] == AssetRefCharTableSize )
		{
			CharIndexes[i] = 0;
		}
		else
		{
			CharIndexes[i]++;
		}

		Changed = TRUE;
	}
	*/

	//	increment last character
	i = AssetRefCharCount-1;
	while ( TRUE )
	{
		if ( CharIndexes[i] == AssetRefCharTableSize )
		{
			CharIndexes[i] = 0;
			i--;
			if ( i<0 )
				i = AssetRefCharCount-1;
		}
		else
		{
			CharIndexes[i]++;
			Changed = TRUE;
			break;
		}
	}


	//	reset ref
	AssetRef = 0x0;

	//	create u32 from char numbers
	BitIndex = 0;
	for ( i=0;	i<AssetRefCharCount;	i++ )
	{
		AssetRef |= ( CharIndexes[i] & AssetCharIndexBitMask ) << BitIndex;
		BitIndex += 6;
	}

	//	still hasnt changed
	if ( OldRef == AssetRef )
	{
		GDebug::Print("Failed to change ref\n");
		AssetRef++;
	}
	

//	GDebug::Popup("Incremented ref from \"%s\" to \"%s\".", GAsset::RefToName(OldRef), GAsset::RefToName( AssetRef ) );

	return AssetRef;
}

	
