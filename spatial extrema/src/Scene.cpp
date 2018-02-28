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
	//Utility::errorExit("exit");
	m_cam = Camera( addEnt() );
	m_cam.setInitPos( ngl::Vec3( 0.0f, 0.0f, 10.0f ) );
	m_cam.setInitPivot( ngl::Vec3( 0.0f, 0.0f, 0.0f ) );
	m_cam.setFOV( 60.0f );
	float aspect = 1024.0f / 720.0f;
	m_cam.setAspect( aspect );
	m_cam.setNear( 0.1f );
	m_cam.setFar( 1024.0f );
	m_cam.clearTransforms();
	m_cam.calculate();

	Light light(
				addEnt(),
				ngl::Vec3( 1.0f, 1.0f, 1.0f ),
				1.0f,
				LIGHT_DIRECTIONAL,
				true
				);
	m_lights.push_back( light );
}

void Scene::draw(const float _dt)
{
	m_renderer.shadingPipeline( "deferred" );
	m_renderer.clear();
	m_renderer.shader( "deferred_pass" );
	m_renderer.draw( "sphere", ngl::Vec3(0,0,0), ngl::Vec3(0,0,0), true );

	m_renderer.render();
	m_renderer.swap();
	//m_renderer.debug();
	//Utility::errorExit( std::to_string( glGetError() ) );
}

void Scene::update(const float _dt)
{
	m_cam.calculate();
	m_renderer.setCamera( &m_cam );
	m_renderer.setLights( &m_lights );
	m_renderer.update();
}

MemRef<PhysEnt> Scene::addEnt()
{
	m_ents.push_back( PhysEnt() );
	return MemRef<PhysEnt>( m_ents.backID() );
}

