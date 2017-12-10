//----------------------------------------------------------------------------------------------------------------------
/// \file light.hpp
/// \brief This class acts as a simple point light.
/// \author Ben Hawkyard
/// \version 1.0
/// \date 19/01/17
/// Revision History :
/// This is an initial version used for the program.
/// \class light
/// \brief Contains vec4 position for transformation, a vec3 colour and a float opacity/brightness.
//----------------------------------------------------------------------------------------------------------------------

#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <ngl/Vec3.h>

#include "MemRef.hpp"
#include "PhysEnt.hpp"

enum LightType {LIGHT_POINT, LIGHT_SPOT, LIGHT_DIRECTIONAL};

struct RenderLight
{
	//A light the renderer can work with, with pared-down data.
	ngl::Vec4 m_pos;
	ngl::Vec4 m_dir;
	ngl::Vec4 m_col;
	int m_type;
};

struct Light
{
	/// @brief Constructor for the light class, takes a position, an RGB colour and a brightness.
	Light(
			const MemRef< PhysEnt > &_base,
			const ngl::Vec3 &_col,
			const float _lum,
			const LightType _lightType = LIGHT_POINT,
			const bool _shadowCasting = false
			)
	{
		m_base = _base;
		m_col = _col;
		m_lum = _lum;
		m_lightType = _lightType;
		m_shadowCasting = _shadowCasting;
	}

	RenderLight getRenderData()
	{
		RenderLight r;
		r.m_pos = m_base->getPos();
		r.m_dir = m_base->forward();
		r.m_col = ngl::Vec4(
					m_col.m_x,
					m_col.m_y,
					m_col.m_z,
					m_lum
					);
		r.m_type = static_cast<int>(m_lightType);
		return r;
	}

	MemRef< PhysEnt > m_base;
	ngl::Vec3 getRot() { return m_base->getRot(); }

	/// @brief The colour of the light.
	ngl::Vec3 m_col;

	/// @brief The brightness of the light.
	float m_lum;

	LightType m_lightType;

	/// @brief Whether the light casts shadows.
	bool m_shadowCasting;
};

#endif // LIGHT_HPP
