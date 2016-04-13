/*------------------------------------------------

  GList Header file

	dynamic array that reallocates data when the list grows or shrinks

-------------------------------------------------*/

#ifndef __GLIST__H_
#define __GLIST__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GDebug.h"
#include "GListTemplate.h"

//	Macros
//------------------------------------------------
#define GLIST_DEFAULT_GROWBY	10	//	when we have to increase our array, alloc a block at once to save lots of creation and deletion



//	Types
//------------------------------------------------

template <class TYPE>
class GList : public GListTemplate<TYPE>
{
protected:
	int					m_Alloc;	//	memory allocated
	int					m_Size;		//	current amount used
	TYPE*				m_pData;	//	pointer to actual data
	int					m_GrowBy;	//	amount to growby at once

public:
	GList();
	GList(const GList<TYPE>& CopyList);								//	construct from another list of this type
	~GList()														{	Realloc(0);	};
	
	virtual int			Size() const								{	return m_Size;	};
	virtual TYPE*		Data() const								{	return m_pData;	};
	virtual const TYPE*	DataConst() const							{	return m_pData;	};
	inline void			SetGrow(int Grow)							{	m_GrowBy = Grow;	};

	//	array
	virtual void		Resize(int size);							//	set new size
	virtual void		Realloc(int size);							//	set new amount of allocated data

	virtual TYPE&		ElementAt(int Index)						{	GDebug_CheckIndex(Index,0,m_Size); return m_pData[Index];	};
	virtual const TYPE&	ElementAtConst(int Index) const				{	GDebug_CheckIndex(Index,0,m_Size); return m_pData[Index];	};
	
	//	operators
	inline TYPE&		operator[](int Index)						{	return ElementAt(Index);	}
	inline TYPE&		operator[](u32 Index)						{	return ElementAt(Index);	}
	inline const TYPE&	operator[](int Index) const					{	return ElementAtConst(Index);	}
	inline const TYPE&	operator[](u32 Index) const					{	return ElementAtConst(Index);	}
	inline void			operator+=(const GList<TYPE>& Array)		{	Add(Array);	};
	inline void			operator+=(const TYPE val)					{	Add(val);	};
	inline void			operator+=(const TYPE& val)					{	Add(val);	};
	inline void			operator+=(const TYPE* val)					{	Add(val);	};
};			




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------

template <class TYPE>
GList<TYPE>::GList()
{
	m_Alloc		= 0;
	m_Size		= 0;
	m_pData		= NULL;
	m_GrowBy	= GLIST_DEFAULT_GROWBY;
	m_Sorted	= FALSE;
}


template <class TYPE>
GList<TYPE>::GList(const GList<TYPE>& CopyList)
{
	GList();
	Copy(CopyList);
}



//-------------------------------------------------------------------------
//	set a new size for the array
//-------------------------------------------------------------------------
template <class TYPE>
void GList<TYPE>::Resize(int size)
{
	//	check param
	if ( size < 0 )
		size = 0;

	//	no change in size
	if ( m_Size == size )
		return;

	//	16-01-04 changed so we just set the size, ReAlloc(0) must be called to clean up
	/*
	if ( size<=0 )
	{
		//	delete all data
		Realloc(0);
		return;
	}
	*/

	if ( (int)size<=m_Alloc )
	{
		//	dont need to expand our array
		m_Size = (int)size;

		if ( m_Size < 2 )
			SetSorted(TRUE);
		
		return;
	}

	/*
	TYPE* pOldData = m_pData;
	int OldLen = m_Size;

	//	pad m_Alloc up to growby rate
	if ( !m_GrowBy )
		m_GrowBy = 1;
	int Extra	= m_GrowBy - (size%m_GrowBy);
	m_Alloc = (int)size + Extra;
	m_Size	= (int)size;

	//	alloc new data
	m_pData	= new TYPE[ m_Alloc ];

	//	copy old elements
	memcpy( m_pData, pOldData, sizeof(TYPE) * OldLen );
	memset( &m_pData[OldLen], 0, sizeof(TYPE) * (m_Alloc - OldLen) );

	//	delete old data
	DELETE_ARRAY(pOldData);
	*/
	Realloc( size );
	m_Size = size;

	//	list must be sorted if there will be 1 or less elements, otherwise assume new elements will make it out of order
	SetSorted(size<2);
}





template <class TYPE>
void GList<TYPE>::Realloc(int size)
{
	//	0 size specified delete all data
	if ( size <= 0 )
	{
		m_Alloc = 0;
		m_Size	= 0;
		GDeleteArray( m_pData );
		return;
	}

	//	no change in size
	if ( size == m_Alloc )
		return;

	//	new allocation
	if ( m_Alloc == 0 )
	{
		m_Alloc = size;
		m_Alloc += m_GrowBy;
		//m_Alloc %= m_GrowBy;
		m_pData	= new TYPE[ m_Alloc ];
		
		#ifdef MEMSET_NEW_ALLOC
			memset( m_pData, 0, sizeof(TYPE) * (m_Alloc) );
		#endif

		return;
	}

	//	resizing allocation
	TYPE* pOldData = m_pData;

	//	pad m_Alloc up to growby rate
	if ( !m_GrowBy )
		m_GrowBy = 1;

	//	set size and align to growby
	m_Alloc = size;
	m_Alloc += m_GrowBy;
	//m_Alloc %= m_GrowBy;

	//	alloc new data
	m_pData	= new TYPE[ m_Alloc ];
	if ( !m_pData )
	{
		GDebug_Break("Failed to allocate %d elements for GList\n",m_Alloc);
		Realloc(0);
		return;
	}

	//	copy old elements
	if ( pOldData )
	{
		GCopyData( m_pData, pOldData, m_Size );
	}

	//	delete old data
	GDeleteArray( pOldData );
}






#endif

