#ifndef __SCENE_HPP__
#define __SCENE_HPP__

#include <memory>

#include <ngl/Transformation.h>

#include "AssetStore.hpp"
#include "Camera.hpp"
#include "Framebuffer.hpp"
#include "Light.hpp"
#include "PhysEnt.hpp"
#include "Renderer.hpp"
#include "Slotmap.hpp"

class Scene
{
public:
	Scene();
	void draw( const float _dt );
	void update( const float _dt );
	bool done() const {return false;}

	SlotID<PhysEnt> addEnt();
private:
	Renderer m_renderer;
	Camera m_cam;
	Slotmap< PhysEnt > m_ents;
};

#endif//__SCENE_HPP__
