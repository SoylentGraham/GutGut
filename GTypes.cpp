/*------------------------------------------------

  GTypes.cpp

	Basic types that wont need to use other more complicated types


-------------------------------------------------*/


//	Includes
//------------------------------------------------
#include "GTypes.h"
#include "GDebug.h"

//	globals
//------------------------------------------------



//	Definitions
//------------------------------------------------


void GDebug::CheckFloat(float2& f2)					
{	
	CheckFloat(f2[0]);	
	CheckFloat(f2[1]);	
}

void GDebug::CheckFloat(float3& f3)
{	
	CheckFloat(f3[0]);	
	CheckFloat(f3[1]);	
	CheckFloat(f3[2]);	
}

void GDebug::CheckFloat(float4& f4)					
{	
	CheckFloat(f4[0]);	
	CheckFloat(f4[1]);	
	CheckFloat(f4[2]);	
	CheckFloat(f4[3]);
}



float3 GetDebugColour(int ColourIndex,GDebugColourBase ColourBase)
{
	//	base colours
	#define COLOUR_MASKS			ARRAY_SIZE(g_ColourMasks)
	#define COLOUR_MASK_STRENGTH	0.6f
	#define CS	COLOUR_MASK_STRENGTH

	const float3 g_ColourMasks[] = 
	{
		float3( CS,	0,	0 ),	//	red
		float3( 0,	CS,	0 ),	//	green
		float3( 0,	0,	CS ),	//	blue
		float3( CS,	CS,	0 ),	//	yellow
		float3( CS,	0,	CS ),	//	purple/pink
		float3( 0,	CS,	CS ),	//	cyan
	};


	//	pick a base colour
	int ColourBaseIndex;

	if ( ColourBase == GDebugColourBase_Any )
		ColourBaseIndex = ColourIndex % COLOUR_MASKS;
	else
		ColourBaseIndex = ColourBase - 1;

	//	get that base colour for maniuplation
	float3 Colour( g_ColourMasks[ColourBaseIndex] );

	#define MAX_VARIANTS	10

	//	pick a variant of the colour from the number, not including the base index
//	int Variant = (ColourIndex - ColourBase) % MAX_VARIANTS;
	int Variant = (ColourIndex - COLOUR_MASKS) % MAX_VARIANTS;

	float ColourMod = 1.f-( (float)Variant / (float)MAX_VARIANTS );
	Colour *= ColourMod;

	return Colour;
}


