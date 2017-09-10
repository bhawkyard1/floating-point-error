//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file contains work developed as a part of
// my Year 2 Computing for Animation 1 project, with
// some large alterations to functionality. It
// may not be appropriate to consider it.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <ciso646>

#include "slotmap.hpp"

bool operator ==(const Slot &_lhs, const Slot &_rhs)
{
    return _lhs.m_id == _rhs.m_id and _lhs.m_version == _rhs.m_version;
}

bool operator !=(const Slot &_lhs, const Slot &_rhs)
{
    return _lhs.m_id != _rhs.m_id or _lhs.m_version != _rhs.m_version;
}
