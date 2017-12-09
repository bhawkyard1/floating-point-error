//A physically manifested entity, with position, rotation and associated velocities.

#ifndef PHYSENT
#define PHYSENT

#include <ngl/Vec3.h>

class PhysEnt
{
public:
	PhysEnt() = default;
	PhysEnt( const PhysEnt &_rhs ) = default;
	~PhysEnt() = default;

	void setPos( const ngl::Vec3 &_pos ) {m_pos = _pos;}
	void incrPos( const ngl::Vec3 &_pos) {m_pos += _pos;}
	ngl::Vec3 getPos() const {return m_pos;}

	void setVel( const ngl::Vec3 &_vel ) {m_vel = _vel;}
	void incrVel( const ngl::Vec3 &_vel ) {m_vel += _vel;}
	ngl::Vec3 getVel() const {return m_vel;}

	void setRot( const ngl::Vec3 &_rot ) {m_rot = _rot;}
	void incrRot( const ngl::Vec3 &_rot ) {m_rot += _rot;}
	ngl::Vec3 getRot() const {return m_rot;}

	void setRotVel( const ngl::Vec3 &_rotVel ) {m_rotVel = _rotVel;}
	ngl::Vec3 incrRotVel( const ngl::Vec3 &_rotVel ) {m_rotVel += _rotVel;}
	ngl::Vec3 getRotVel() const {return m_rotVel;}

	ngl::Vec3 up() const;
	ngl::Vec3 right() const;
	ngl::Vec3 forward() const;

	ngl::Vec3 down() const {return -up();}
	ngl::Vec3 left() const {return -right();}
	ngl::Vec3 back() const {return -forward();}

	void update(const float _dt);

private:
	ngl::Vec3 m_pos;
	ngl::Vec3 m_vel;

	ngl::Vec3 m_rot;
	ngl::Vec3 m_rotVel;
};

#endif
