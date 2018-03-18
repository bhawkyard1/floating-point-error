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
	m_renderer.setCamera( &m_cam );

	float SPEED = 10000.0f;

	Light light1(
				addEnt(),
				ngl::Vec3( 1.0f, 0.0f, 0.0f ),
				32.0f,
				LIGHT_DIRECTIONAL,
				false
				);
	light1.m_base->setParent( m_cam.getBase() );
	m_lights.push_back( light1 );

	Light light2(
				addEnt(),
				ngl::Vec3( 0.0f, 1.0f, 0.0f ),
				32.0f,
				LIGHT_DIRECTIONAL,
				false
				);
	light2.m_base->setParent( m_cam.getBase() );
	m_lights.push_back( light2 );

	Light light3(
				addEnt(),
				ngl::Vec3( 0.0f, 0.0f, 1.0f ),
				32.0f,
				LIGHT_DIRECTIONAL,
				false
				);
	light3.m_base->setParent( m_cam.getBase() );
	m_lights.push_back( light3 );

	m_cam.getBase()->setPos(ngl::Vec3(0.0f, 0.0f, 510.0f));
	m_cam.getBase()->setVel(ngl::Vec3(0.0f, 0.0f, SPEED));

	MemRef<RenderEnt> model = addRenderEnt();
	model->setModel( "david" );
	MemRef<PhysEnt> modelPhysical = addEnt();
	modelPhysical->setRenderEnt( model );
	modelPhysical->setVel(ngl::Vec3(0.0f, 0.0f, SPEED));
	modelPhysical->setRotVel(ngl::Vec3(0.0f, 0.1f, 0.05f));
}

void Scene::draw(const float _dt)
{
	m_renderer.framebuffer( "deferred" );
	m_renderer.clear();
	m_renderer.shader( "deferred_pass" );
	//m_renderer.draw( "david", ngl::Vec3(0,0,0), ngl::Vec3(0,0,0), true );

	for(auto &i : m_rents.m_objects)
	{
		if(i.getShader() != "")
			m_renderer.shader( i.getShader() );
		m_renderer.draw( i.getModel(), i.getTransform(), false );
	}

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

	std::cout << m_cam.getBase()->getPos() << '\n';

	for( auto &i : m_ents.m_objects )
		i.update( _dt );
}

MemRef<PhysEnt> Scene::addEnt()
{
	m_ents.push_back( PhysEnt() );
	return MemRef<PhysEnt>( m_ents.backID() );
}

MemRef<RenderEnt> Scene::addRenderEnt()
{
	m_rents.push_back( RenderEnt() );
	return MemRef<RenderEnt>( m_rents.backID() );
}

