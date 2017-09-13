#ifndef MEMREF_HPP
#define MEMREF_HPP

#include "Slotmap.hpp"

enum RefType {REF_NONE, REF_RAW, REF_SLOT};

template< typename T >
class MemRef
{
public:
	MemRef()
	{
		m_type = REF_NONE;
		m_ref.m_raw = nullptr;
	}

	MemRef( T * _ref )
	{
		m_type = REF_RAW;
		m_ref.m_raw = _ref;
	}

	MemRef( SlotID<T> _ref )
	{
		m_type = REF_SLOT;
		m_ref.m_slot = _ref;
	}

	MemRef( const MemRef<T> &_rhs ) = default;

	T * get()
	{
		switch( m_type )
		{
		case REF_RAW:
			return m_ref.m_raw;
			break;
		case REF_SLOT:
			return m_ref.m_slot.get();
			break;
		}
		return nullptr;
	}

	T& operator* ()
	{
		return get();
	}

	T* operator-> ()
	{
		switch( m_type )
		{
		case REF_RAW:
			return m_ref.m_raw;
			break;
		case REF_SLOT:
			return m_ref.m_slot.get();
			break;
		case REF_NONE:
			return nullptr;
			break;
		}
		return nullptr;
	}

	bool isNull() const {return m_type == REF_NONE;}
private:
	RefType m_type;
	union MemRefStorage {
		MemRefStorage() :
			m_raw( nullptr ),
			m_slot()
		{}

		T * m_raw;
		SlotID< T > m_slot;
	} m_ref;
};

#endif
