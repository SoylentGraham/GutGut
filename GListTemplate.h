/*------------------------------------------------

  GListTemplate Header file

	template for list classes, implements the basic list 
	stuff without duplicating code


-------------------------------------------------*/

#ifndef __GLISTTEMPLATE__H_
#define __GLISTTEMPLATE__H_



//	Includes
//------------------------------------------------
#include "GMain.h"
#include "GDebug.h"


//	Macros
//------------------------------------------------




//	Types
//------------------------------------------------

//-------------------------------------------------------------------------
//	template class for list classes, both fixed and dynamic.
//	functions marked with //T need to be implemented
//-------------------------------------------------------------------------
template <class TYPE>
class GListTemplate
{
protected:
	Bool				m_Sorted;	//	set to true everytime the list is sorted. if any elemets are added or removed, this becomes invalid

public:
	GListTemplate();
	
	virtual int			Size() const=0;
	inline int			LastIndex() const					{	return Size() - 1;	};
	virtual TYPE*		Data() const=0;
	virtual const TYPE*	DataConst() const=0;
	inline int			DataSize() const					{	return ( Size() * sizeof(TYPE) );	};
	TYPE*				CopyData() const;					//	makes a copy of the data and returns a pointer to it

	//	array
	virtual void		Copy(const GListTemplate<TYPE>& Array);		//	make this a copy of the specified array
	void				CopyElements(const TYPE* pData, int Length, int Index );
	void				SetAll(const TYPE& Val);			//	set all elements to match this one (uses = operator)

	virtual void		Resize(int size)=0;					//	set new size
	virtual void		Realloc(int size)=0;				//	set new amount of allocated data
	void				Empty()								{	Resize(0);	};
	void				ShiftArray(int From, int Amount );	//	move blocks of data in the array

	virtual TYPE&		ElementAt(int Index)=0;
	virtual const TYPE&	ElementAtConst(int Index) const=0;
	inline TYPE&		ElementLast()						{	GDebug_CheckIndex(LastIndex(),0,Size());	return ElementAt(LastIndex());	};
	inline const TYPE&	ElementLastConst() const			{	GDebug_CheckIndex(LastIndex(),0,Size());	return ElementAtConst(LastIndex());	};

	virtual int			Add(const TYPE& val);				//	add an element onto the end of the list
	virtual int			Add(const TYPE* val,int Length=1);	//	add a number of elements onto the end of the list
	virtual int			Add(const GListTemplate<TYPE>& Array);	//	add a whole array of this type onto the end of the list
	virtual Bool		RemoveAt(int Index);				//	remove an element from the array at the specified index
	Bool				RemoveLast()						{	return ( Size() ? RemoveAt( LastIndex() ) : FALSE );	};
	virtual int			Insert(int Index, const TYPE& val,BOOL ForcePosition=FALSE);
	virtual int			Insert(int Index, const TYPE* val, int Length, BOOL ForcePosition=FALSE);
	virtual int			Insert(int Index, const GListTemplate<TYPE>& array, BOOL ForcePosition=FALSE)		{	return Insert( Index, array.Data(), array.Size(), ForcePosition );	};
	virtual void		Move(int CurrIndex,int NewIndex);	//	remove from list in one place and insert it back in
	inline void			Swap(int a, int b);					//	swap 2 elements in the array
	inline void			Sort(Bool RemoveDuplicates=FALSE);	//	quick sort the list. optional flag to remove duplicates at the same time
	inline Bool			Sorted() const 						{	return m_Sorted;	};	//	is the list sorted?
	
	template<class MATCHTYPE>
	int					FindIndex(const MATCHTYPE& val) const
	{
		//if ( Sorted() )	
		//	return FindIndexBinaryChop(val);
		for ( int i=0;	i<Size();	i++ )
		{
			if ( ElementAtConst(i) == val )
				return i;
		}
		return -1;
	};

	template<class MATCHTYPE>
	inline TYPE*		Find(const MATCHTYPE& val)				{	int Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAt(Index);	};

	template<class MATCHTYPE>
	inline const TYPE*	FindConst(const MATCHTYPE& val) const	{	int Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAtConst(Index);	};

	template<class MATCHTYPE>
	inline BOOL			Exists(const MATCHTYPE& val)			{	return FindIndex(val)!=-1;	};

	//	operators
	inline TYPE&		operator[](int Index)					{	return ElementAt(Index);	}
	inline TYPE&		operator[](u32 Index)					{	return ElementAt(Index);	}
	inline const TYPE&	operator[](int Index) const				{	return ElementAt(Index);	}
	inline const TYPE&	operator[](u32 Index) const				{	return ElementAt(Index);	}
	inline void			operator+=(const GListTemplate<TYPE>& Array)	{	Add(Array);	};
	inline void			operator+=(const TYPE val)				{	Add(val);	};
	inline void			operator+=(const TYPE& val)				{	Add(val);	};
	inline void			operator+=(const TYPE* val)				{	Add(val);	};

protected:
	void				QuickSort(int First, int Last, Bool RemoveDuplicates);
	inline void			SetSorted(Bool IsSorted)				{	m_Sorted = IsSorted;	};			//	called when list order changes

	template<class MATCHTYPE>
	int					FindIndexBinaryChop(const MATCHTYPE& val) const
	{
		GDebug_Break("Binary chop not implemented yet\n");
		return -1;
	}
};			




//	Declarations
//------------------------------------------------




//	Inline Definitions
//-------------------------------------------------

template <class TYPE>
GListTemplate<TYPE>::GListTemplate()
{
	m_Sorted	= FALSE;
}



template <class TYPE>
TYPE* GListTemplate<TYPE>::CopyData() const
{
	if ( !Size() )
		return NULL;

	TYPE* pCopy = new TYPE[Size()];
	memcpy( pCopy, Data(), sizeof(TYPE) * Size() );

	//	list must be sorted if there will be 1 or less elements, otherwise assume new elements will make it out of order
	SetSorted(size<2);
	
	return pCopy;
}



template <class TYPE>
int GListTemplate<TYPE>::Add(const TYPE& val)
{
	Resize(Size()+1);
	ElementLast() = val;

	//	list may no longer be sorted
	SetSorted(FALSE);

	return (int)LastIndex();
}


	
template <class TYPE>
int GListTemplate<TYPE>::Add(const TYPE* val,int Length)
{	
	if( val == NULL )
	{
		GDebug_Break("Null pointer specified in add\n");
		return -1;
	}

	int First = Size();
	Resize(Size()+Length);
	CopyElements( val, Length, First );

	//	list may no longer be sorted
	SetSorted(FALSE);

	return First;	
};


//-------------------------------------------------------------------------
//	remove an element based on its index. doesnt affect sorted state
//-------------------------------------------------------------------------
template <class TYPE>
Bool GListTemplate<TYPE>::RemoveAt(int Index)
{
	GDebug_CheckIndex(Index,0,Size());

	if ( Index>LastIndex() )
		return FALSE;
	ShiftArray(Index,-1);

	return TRUE;
}



template <class TYPE>
int GListTemplate<TYPE>::Insert(int Index, const TYPE& val, BOOL ForcePosition)
{
	//	need to add it onto the end
	if ( Index > LastIndex() )
	{
		if ( ForcePosition )
		{
			//	expand list to fit index
			Resize(Index+1);
		}
		else
		{
			//	set index so its at the end
			Resize( Size()+1 );
			Index = LastIndex();
		}
	}
	else
	//	dont need to shift if its the last index
	if ( Index <= LastIndex() )
	{
		ShiftArray(Index,1);
	}

	//	set the val
	ElementAt(Index) = val;

	//	list may no longer be sorted
	SetSorted(FALSE);

	return Index;
}


template <class TYPE>
int GListTemplate<TYPE>::Insert(int Index, const TYPE* val, int Length, BOOL ForcePosition)
{
	//	need to add it onto the end
	if ( Index > LastIndex() )
	{
		if ( ForcePosition )
		{
			//	expand list to fit index
			Resize(Index+Length);
		}
		else
		{
			//	set index so its at the end
			Index = Size();
			Resize( Size()+Length );
			//Index = LastIndex();
		}
	}
	else if ( Index <= LastIndex() )
	{
		//	dont need to shift if its the last index
		ShiftArray(Index,Length);
	}

	//	set the values
	CopyElements( val, Length, Index);

	return Index;
}








template <class TYPE>
int GListTemplate<TYPE>::Add(const GListTemplate<TYPE>& Array)
{
	if ( Array.Size () )
	{
		Add( Array.Data(), Array.Size() );
	}
	return Size();
}


template <class TYPE>
void GListTemplate<TYPE>::Copy(const GListTemplate<TYPE>& Array)
{
	Resize(Array.Size());

	CopyElements( Array.Data(), Array.Size(), 0 );
}

//-------------------------------------------------------------------------
//	copy Length elements from another source into our array at Index
//-------------------------------------------------------------------------
template <class TYPE>
void GListTemplate<TYPE>::CopyElements(const TYPE* pData, int Length, int Index )
{
	//	todo: check for size/allocation errors
	if ( Length < 1 )
		return;

	//	copy data
	GCopyData( &ElementAt(Index), pData, Length );

	//	list may no longer be sorted
	SetSorted(FALSE);
}


template <class TYPE>
void GListTemplate<TYPE>::Move(int CurrIndex,int NewIndex)
{
	TYPE Item = ElementAt(CurrIndex);
	RemoveAt(CurrIndex);
	Insert(NewIndex,Item);

	//	list may no longer be sorted
	SetSorted(FALSE);
}





template <class TYPE>
void GListTemplate<TYPE>::ShiftArray(int From, int Amount )
{
	if ( Amount == 0 )	
		return;
	
	if ( Amount > 0 )
	{
		Resize(Size()+Amount);
		for ( int i=LastIndex();	i>From;	i-- )
		{
			int FromIndex = i - Amount;
			if ( FromIndex < 0 )
			{
				//	hmmmm
			}
			else
			{
				ElementAt(i) = ElementAt(FromIndex);
			}
		}
	} 
	else if ( Amount < 0 )
	{
		for ( int i=From;	i<Size()-1;	i++ )
			ElementAt(i) = ElementAt(i-Amount);	//	+shift
		Resize(Size()+Amount);
	}

	//	list may no longer be sorted
	SetSorted(FALSE);
}






//-------------------------------------------------------------------------
//	swap order of two elements
//-------------------------------------------------------------------------
template <class TYPE>
void GListTemplate<TYPE>::Swap(int a, int b)
{
	TYPE Tmp = ElementAt(a);
	ElementAt(a) = ElementAt(b);
	ElementAt(b) = Tmp;

	//	list may no longer be sorted
	SetSorted(FALSE);
}



//-------------------------------------------------------------------------
//	make all elements the same
//-------------------------------------------------------------------------
template <class TYPE>
void GListTemplate<TYPE>::SetAll(const TYPE& Val)
{
	//	if we can use memset, use it
/*	if ( sizeof(TYPE) == 1 )	//	1 byte
	{
		memset( Data(), Val, Size() );
	}
	else
*/	{
		for ( int i=0;	i<Size();	i++ )
		{
			memcpy( &ElementAt(i), &Val, sizeof(TYPE) );
		}
	}

	//	list is no longer sorted
	SetSorted(FALSE);
}


//-------------------------------------------------------------------------
//	initial sorting func does stuff that only needs to be done once
//-------------------------------------------------------------------------
template <class TYPE>
inline void GListTemplate<TYPE>::Sort(Bool RemoveDuplicates)
{
	//	already sorted or nothing to sort
	if ( m_Sorted || Size() < 2 )
	{
		SetSorted( TRUE );
		return;
	}

	//	do sort
	QuickSort( 0, Size()-1, RemoveDuplicates );

	//	we're now sorted!
	SetSorted( TRUE );
}


//-------------------------------------------------------------------------
//	Quicksort recursive func
//-------------------------------------------------------------------------
template <class TYPE>
void GListTemplate<TYPE>::QuickSort(int First, int Last, Bool RemoveDuplicates)
{
	//	check params
	if ( First >= Last )	return;
	if ( First == -1 )		return;
	if ( Last == -1 )		return;

	//	check for duplicates
	if ( RemoveDuplicates )
	{
		if ( First != Last && ElementAt(First) == ElementAt(Last) )
		{
			//	remove this duplicate and attempt sort again
			RemoveAt( Last );
			QuickSort( First, Last-1, RemoveDuplicates );	//	need -1?
			return;
		}
	}

	int End = First;
	for ( int Current=First+1;	Current<=Last;	Current++ )
	{
		if ( CompareIsLess( Current, First ) )
		{
			Swap( ++End, Current );
		}
	}

	Swap( First, End );
	QuickSort( First, End-1, RemoveDuplicates );
	QuickSort( End+1, Last, RemoveDuplicates );
}





#endif // __GLISTTEMPLATE__H_

