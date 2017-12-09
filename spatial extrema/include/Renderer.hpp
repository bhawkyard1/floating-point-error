#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>

#include <ngl/Transformation.h>
#include <ngl/Vec2.h>

#include "AssetStore.hpp"
#include "Camera.hpp"
#include "Framebuffer.hpp"
#include "PhysEnt.hpp"
#include "ShadingPipeline.hpp"
#include "Light.hpp"

typedef std::pair<ngl::Vec3, ngl::Vec3> bounds;

class Renderer
{
public:
	Renderer( const ngl::Vec2 _dimensions );

	void createShader(const std::string _name, const std::string _vert, const std::string _frag, const std::string _geo = "", const std::string _tessctrl = "", const std::string _tesseval = "");
	std::string loadShaderToString( const std::string &_path );

	template<typename T>
	void setCamera( T _cam )
	{
		m_cam = MemRef<Camera>( _cam );
	}

	//TODO delete
	void debug();
	Framebuffer m_debugBuffer;
	std::vector<GLuint> m_debugIds;

	void shader( const std::string &_shader );
	//Draws something into the current shading pipeline.
	void draw(const std::string &_mesh,
						const ngl::Vec3 &_pos = ngl::Vec3(0.0f, 0.0f, 0.0f),
						const ngl::Vec3 &_rot = ngl::Vec3(0.0f, 0.0f, 0.0f),
						const bool _shadows = false);
	//Runs a shading pipeline and outputs an image.
	void render();

	void shadingPipeline( const std::string &_pipe );

	void clear();
	void swap();

	GLuint createVAO(std::vector<ngl::Vec4> &_verts);
	GLuint createVAO(std::vector<ngl::Vec4> &_verts, std::vector<ngl::Vec2> &_uvs);
	GLuint createVAO(std::vector<ngl::Vec4> &_verts, std::vector<ngl::Vec3> &_normals, std::vector<ngl::Vec2> &_uvs);
	GLuint createBuffer4f(std::vector<ngl::Vec4> _vec);
	GLuint createBuffer3f(std::vector<ngl::Vec3> _vec);
	GLuint createBuffer2f(std::vector<ngl::Vec2> _vec);
	GLuint createBuffer1f(std::vector<float> _vec);
	void setBufferLocation(GLuint _buffer, int _index, int _size);

	///
	/// @brief Loads a custom set of matrices to the active shader
	/// @param _M, The model matrix.
	/// @param _MVP, The model-view-project matrix.
	///
	void loadMatricesToShader(const ngl::Mat4 _M, const ngl::Mat4 _MVP);
	///
	/// @brief Loads the MVP matrix to the active shader.
	///
	void loadMatricesToShader();

private:
	MemRef< Camera > m_cam;

	//Assets
	static AssetStore s_assetStore;

	GLuint m_screenQuadVAO;

	//Windowing
	SDL_Window * m_window;
	SDL_GLContext m_glContext;
	ngl::Vec2 m_dimensions;

	//Framebuffers
	Slotmap< Framebuffer > m_framebuffers;

	//Shading pipelines.
	std::unordered_map< std::string, ShadingPipeline > m_pipelines;

	ngl::Transformation m_transform;
};

#endif
