#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Random.h>

#include "AssetStore.hpp"
#include "Scene.hpp"
#include "Utility.hpp"

#include <ngl/NGLStream.h>

#include <ngl/AABB.h>

Scene::Scene() :
	m_renderer( ngl::Vec2( 1024.0f, 720.0f ) )
{
	m_cam = Camera( addEnt() );
	m_cam.setInitPos( ngl::Vec3( 0.0f, 0.0f, 10.0f ) );
	m_cam.setInitPivot( ngl::Vec3( 0.0f, 0.0f, 0.0f ) );

	float aspect = 1024.0f / 720.0f;
	m_cam.setAspect( aspect );
	m_cam.clearTransforms();
	m_cam.calculate();
}

void Scene::draw(const float _dt)
{
	m_renderer.shadingPipeline( "deferred" );
	m_renderer.clear();
	//m_renderer.shader( "deferred_pass" );
	//m_renderer.draw( "david" );

	m_renderer.render();
	m_renderer.swap();
}

void Scene::update(const float _dt)
{
	m_renderer.setCamera( &m_cam );
}

SlotID<PhysEnt> Scene::addEnt()
{
	m_ents.push_back( PhysEnt() );
	return m_ents.backID();
}

