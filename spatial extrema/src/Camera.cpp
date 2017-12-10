#include <array>

#include <ngl/Util.h>
#include <ngl/Vec3.h>

#include "Camera.hpp"
#include "Utility.hpp"

#include <iostream>
#include <ngl/NGLStream.h>

Camera::Camera( MemRef<PhysEnt> _base ) :
	m_base( _base )
{

}

void Camera::calculate()
{
		calculateV();
		calculateP();
    m_VP = m_V * m_P;
}

void Camera::calculateV()
{
		m_cameraTransformation = ngl::Mat4();
		m_worldTransformation = ngl::Mat4();

		for(auto &i : m_cameraTransformationStack)
				m_cameraTransformation *= i;
		for(auto &i : m_worldTransformationStack)
				m_worldTransformation *= i;

		ngl::Vec3 up = ngl::Vec3( 0.0f, 1.0f, 0.0f );
		PhysEnt b =* m_base.get();
		std::cout << b.back().m_x << '\n';
		if( !m_base.isNull() )
			up = m_base->up();

		m_V = ngl::lookAt(
								m_initPos,
								m_initPivot,
								up
								);

		/*for(auto i = m_transformationStack.rbegin(); i < m_transformationStack.rend(); ++i)
				m_V *= (*i);*/

		m_V *= m_cameraTransformation;
		m_V *= m_worldTransformation;

		ngl::Mat4 VI = m_V.inverse();
		ngl::Mat4 wti = m_worldTransformation.inverse();
		m_base->setPos( ngl::Vec3(VI.m_30, VI.m_31, VI.m_32) );
		m_pivot = ngl::Vec3(wti.m_30, wti.m_31, wti.m_32);
}

void Camera::calculateP()
{
    m_P = ngl::perspective(
                m_fov,
                m_aspect,
								m_near,
								m_far
                );
}

void Camera::rotateCamera(const float _pitch, const float _yaw, const float _roll)
{
    m_cameraTransformationStack.push_back( rotationMatrix(_pitch, _yaw, _roll) );
}

void Camera::rotateWorld(const float _pitch, const float _yaw, const float _roll)
{
    m_worldTransformationStack.push_back( rotationMatrix(_pitch, _yaw, _roll) );
}

void Camera::moveCamera(const ngl::Vec3 _translation)
{
    m_cameraTransformationStack.push_back( translationMatrix(_translation) );
}

ngl::Mat4 Camera::rotationMatrix(float _pitch, float _yaw, float _roll)
{
    ngl::Mat4 p;
    ngl::Mat4 y;
    ngl::Mat4 r;

    p.rotateX( _pitch );

    y.rotateY( _yaw );

    r.rotateZ( _roll );

    return y * p;
}

ngl::Mat4 Camera::translationMatrix(const ngl::Vec3 &_vec)
{
    ngl::Mat4 trans;
    trans.translate( _vec.m_x, _vec.m_y, _vec.m_z );
    return trans;
}

std::array<ngl::Vec4, 8> Camera::calculateCascade(float _start, float _end)
{
		std::array<ngl::Vec4, 8> cascade;

		//Distance from the look vector, on the right vector
		float horizontal = tanf( Utility::radians(m_fov) / 1.0f );

		//float verticalFOV = m_fov / m_aspect;

		float verticalFOV = 2.0f * atan( tanf(Utility::radians(m_fov) / 2.0f) / m_aspect );

		float vertical = tanf(verticalFOV);

		_start = -_start;
		_end = -_end;

		float sh = _start * horizontal;
		float sv = _start * vertical;
		float eh = _end * horizontal;
		float ev = _end * vertical;

		//Near plane
		cascade[0] = ngl::Vec4( -sh, sv, _start, 1.0 ); //Top left
		cascade[1] = ngl::Vec4( sh, sv, _start, 1.0 ); //Top right
		cascade[2] = ngl::Vec4( -sh, -sv, _start, 1.0 ); //Bottom left
		cascade[3] = ngl::Vec4( sh, -sv, _start, 1.0 ); //Bottom right

		//Far plane
		cascade[4] = ngl::Vec4( -eh, ev, _end, 1.0 ); //Top left
		cascade[5] = ngl::Vec4( eh, ev, _end, 1.0 ); //Top right
		cascade[6] = ngl::Vec4( -eh, -ev, _end, 1.0 ); //Bottom left.
		cascade[7] = ngl::Vec4( eh, -ev, _end, 1.0 ); //Bottom right

		//Project from view space to world space
		ngl::Mat4 iv = m_V;
		//iv = iv.transpose();
		iv = iv.inverse();

		for(auto &i : cascade)
				i = i * iv;

		//Return cascade in world space.
		return cascade;
}
