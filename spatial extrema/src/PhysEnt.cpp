#include <ngl/Transformation.h>

#include "PhysEnt.hpp"

ngl::Vec3 PhysEnt::up() const
{
	ngl::Transformation t;
	t.setRotation( m_rot );
	return t.getMatrix().getUpVector();
}

ngl::Vec3 PhysEnt::right() const
{
	ngl::Transformation t;
	t.setRotation( m_rot );
	return t.getMatrix().getRightVector();
}

ngl::Vec3 PhysEnt::forward() const
{
	ngl::Transformation t;
	t.setRotation( m_rot );
	return t.getMatrix().getForwardVector();
}

void PhysEnt::update(const float _dt)
{
	m_pos += m_vel * _dt;
	m_rot += m_rotVel * _dt;
}
