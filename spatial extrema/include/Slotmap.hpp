//----------------------------------------------------------------------------------------------------------------------
/// \file Slotmap.hpp
/// \brief This file contains the 'Slotmap' class, and related structs.
/// \author Ben Hawkyard
/// \version 1.0
/// \date 19/01/17
/// Revision History :
/// This is an initial version used for the program.
/// \class Slotmap
/// \brief A wrapper around stl std::vector which allows the user to extract persistant references to entities
/// contained within.
//----------------------------------------------------------------------------------------------------------------------

#ifndef Slotmap_HPP
#define Slotmap_HPP

#ifdef _WIN32
#include <ciso646>
#endif
#include <stddef.h>
#include <vector>

struct Slot
{
	Slot()
	{
		m_id = 0;
		m_version = -1;
	}

	Slot(const long _id, const long _version)
	{
		m_id = _id;
		m_version = _version;
	}

	long m_id;
	long m_version;
};

bool operator ==(const Slot &_lhs, const Slot &_rhs);
bool operator !=(const Slot &_lhs, const Slot &_rhs);

template<class T>
class Slotmap;

template<class T>
struct SlotID : public Slot
{
	SlotID() : Slot()
	{
		m_address = nullptr;
	}

	SlotID(const long _id, const long _version, Slotmap<T> * _src) : Slot(_id, _version)
	{
		m_address = _src;
	}

	Slotmap<T> * m_address;

	T * get() {return m_address->getByID( static_cast< Slot >(*this) );}
};

template<class T>
class Slotmap
{
public:
	//----------------------------------------------------------------------------------------------------------------------
	/// \brief Vector of raw objects.
	//----------------------------------------------------------------------------------------------------------------------
	std::vector<T> m_objects;

	T * getByID(Slot _i)
	{
		//Something wrong here. m_indirection not keeping up with m_ids.
		if(_i.m_id < m_indirection.size() and m_indirection[ _i.m_id ].m_version == _i.m_version)
			return &m_objects[ m_indirection[ _i.m_id ].m_id ];
		return nullptr;
	}

	Slot push_back(const T &_obj)
	{
		Slot ret;
		//If there are free spaces...
		if(m_freeList.size() > 0)
		{
			long freeid = m_freeList.back();
			long ver = m_indirection[ freeid ].m_version;

			m_indirection[ freeid ].m_id = static_cast<long>(m_objects.size());
			m_ids.push_back( {freeid, ver} );
			m_freeList.pop_back();

			ret = {freeid, ver};
		}
		//Create a new object, a new id and a new entry in the indirection list.
		else
		{
			Slot id = {static_cast<long>(m_objects.size()), 0};
			m_indirection.push_back( id );
			m_ids.push_back( id );

			ret = id;
		}

		m_objects.push_back(_obj);

		return ret;
	}

	//Swaps item at index _a with item at index _b
	void swap(long _a, long _b)
	{
		if(_a == _b) return;

		//Store entry pointed to by the id of _a
		Slot swap = {_a, m_ids[_b].m_version};

		//Make the entry at _a's id point to _a's future index.
		m_indirection[ m_ids[_a].m_id ] = {_b, m_ids[_a].m_version};
		//Make the entry at _b's id point to _b's future index.
		m_indirection[ m_ids[_b].m_id ] = swap;

		//std::swap( m_objects[_a], m_objects[_b] );
		//std::swap( m_ids[_a], m_ids[_b] );

		iter_swap( m_objects.begin() + _a, m_objects.begin() + _b );
		iter_swap( m_ids.begin() + _a, m_ids.begin() + _b );
	}

	void pop()
	{
		//Add id to freelist.
		m_freeList.push_back( m_ids.back().m_id );
		//Increment version on indirection list.
		m_indirection[ m_ids.back().m_id ].m_version += 1;

		//Destroy ID.
		m_ids.pop_back();
		//Destroy object.
		m_objects.pop_back();
	}

	void free(size_t _i)
	{
		swap(_i, m_objects.size() - 1);
		pop();
	}

	T& back() const {return m_objects.back();}
	T& back() {return m_objects.back();}
	Slot backSlot() {return m_ids.back();}
	SlotID<T> backID() {return SlotID<T>( m_ids.back().m_id, m_ids.back().m_version, this );}

	void clear()
	{
		m_objects.clear();
		m_ids.clear();
		m_indirection.clear();
		m_freeList.clear();
	}

	size_t size() const {return m_objects.size();}

	Slot getID(size_t _i) const {return m_ids[_i];}
	size_t getIndex(Slot _id) const {return m_indirection[ _id.m_id ].m_id;}

	T operator [](size_t _i) const {return m_objects[_i];}
	T & operator [](size_t _i) {return m_objects[_i];}

	T get(size_t _i) const {return m_objects[_i];}
	T & get(size_t _i) {return m_objects[_i];}

private:
	//----------------------------------------------------------------------------------------------------------------------
	/// \brief The index of each entry is the id of the object. The contents id is the index of the object. The version is the version of the object.
	/// Confused? Me too.
	//----------------------------------------------------------------------------------------------------------------------
	std::vector< Slot > m_indirection;

	//----------------------------------------------------------------------------------------------------------------------
	/// \brief Means we do not have to store ids in the object, this matches movements of m_objects by index.
	//----------------------------------------------------------------------------------------------------------------------
	std::vector< Slot > m_ids;

	//----------------------------------------------------------------------------------------------------------------------
	/// \brief List of all free IDs.
	//----------------------------------------------------------------------------------------------------------------------
	std::vector< long > m_freeList;
};

template<class t>
void transfer(size_t _i, Slotmap<t> &_src, Slotmap<t> &_dst)
{
	_dst.push_back( _src[_i] );
	_src.free( _i );
}



#endif
