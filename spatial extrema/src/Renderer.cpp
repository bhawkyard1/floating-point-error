#include <ngl/NGLInit.h>
#include <ngl/ShaderLib.h>

#include "Renderer.hpp"
#include "File.hpp"
#include "Utility.hpp"

#ifdef _WIN32
#include <ciso646>
#endif

AssetStore Renderer::s_assetStore;

Renderer::Renderer( const ngl::Vec2 _dimensions )
{
	//General setup
	m_cam = nullptr;

	if( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
		Utility::errorExit( "SDL could not be initialised! " + std::string(SDL_GetError()) + '\n' );

	//Window setup
	m_dimensions = _dimensions;
	m_window = SDL_CreateWindow("Spatial Extrema",
															SDL_WINDOWPOS_CENTERED,
															SDL_WINDOWPOS_CENTERED,
															m_dimensions.m_x,
															m_dimensions.m_y,
															SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
															);

	if(m_window == nullptr)
		Utility::errorExit( "Could not create SDL window \n" );

	//Init GL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GLContext m_glContext = SDL_GL_CreateContext( m_window );
	if(!m_glContext)
		Utility::errorExit("Unable to create GL context");
	SDL_GL_MakeCurrent( m_window, m_glContext );
	SDL_GL_SetSwapInterval( 0 );

	ngl::NGLInit::instance();

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glDisable( GL_DEPTH_TEST );

	//Load shaders
	std::vector< std::string > shaders = File::getLinesFromFile( "config/shader-config.rcfg" );
	for( auto &i : shaders )
	{
		if(i.length() == 0 or (i.length() > 0 and i[0] == '#'))
			continue;
		int arglen = 6;
		std::vector< std::string > dat (arglen, "");
		int cnt = 0;
		for( auto &j : Utility::split( i, ' ' ) )
		{
			dat[cnt] = j;
			cnt++;
		}
		createShader( dat[0], dat[1], dat[2], dat[3], dat[4], dat[5] );
	}

	//Load assets.
	std::vector< std::string > assets = File::getLinesFromFile( "config/asset-config.rcfg" );
	for( auto &i : assets )
	{
		if(i.length() == 0 or i.length() > 0 and i[0] == '#')
			continue;

		std::vector< std::string > data = Utility::split( i, ' ' );

		if(data.size() != 3)
			Utility::errorExit( "Error! Invalid asset string, " + i );

		if( data[0] == "MESH" ) s_assetStore.loadMesh( data[2], data[1] );
		else if( data[0] == "TEXTURE" ) s_assetStore.loadTexture( data[2], data[1] );
	}

	//Set up deferred shading pipeline.
	Framebuffer gBuffer;
	gBuffer.initialise(
				m_dimensions.m_x,
				m_dimensions.m_y
				);
	gBuffer.addTexture( "diffuse", GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT0 );
	gBuffer.addTexture( "position", GL_RGBA, GL_RGBA32F, GL_COLOR_ATTACHMENT1 );
	gBuffer.addTexture( "normal", GL_RGBA, GL_RGBA16F, GL_COLOR_ATTACHMENT2 );
	gBuffer.addTexture( "linearDepth", GL_RED, GL_R16F, GL_COLOR_ATTACHMENT3 );
	gBuffer.addDepthAttachment( "depth" );
	if(!gBuffer.checkComplete())
		Utility::errorExit("Uh oh! Framebuffer incomplete! Error code " + glGetError() + '\n');
	gBuffer.unbind();

	m_framebuffers.push_back( gBuffer );
	MemRef< Framebuffer > dataInputRef ( m_framebuffers.backID() );

	Framebuffer compositeBuffer;
	compositeBuffer.initialise(
				m_dimensions.m_x,
				m_dimensions.m_y
				);
	compositeBuffer.addTexture( "solid", GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT0 );
	compositeBuffer.addTexture( "nonsolid", GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT1 );
	compositeBuffer.addDepthAttachment( "depth" );
	if(!compositeBuffer.checkComplete())
		Utility::errorExit("Uh oh! Framebuffer incomplete! Error code " + glGetError() + '\n');
	compositeBuffer.unbind();

	dataInputRef->bind();
	std::cout << "Pre complete " << dataInputRef->checkComplete() << '\n';
	std::cout << "eb3 " << glGetError() << '\n';
	dataInputRef->bind();
	std::cout << "eb4 " << glGetError() << '\n';

	m_framebuffers.push_back( compositeBuffer );
	std::cout << "Post complete " << dataInputRef->checkComplete() << '\n';
	MemRef< Framebuffer > compositeRef ( m_framebuffers.backID() );

	//The stages in our deferred pipeline
	std::vector< ShadingStage > deferredStages;

	//The inputs for the first stage.
	std::vector< ShadingInput > lightingInputs;
	ShadingInput a ( dataInputRef );
	a.m_links.push_back( {"diffuse", "u_diffuse"} );
	a.m_links.push_back( {"position", "u_position"} );
	a.m_links.push_back( {"normal", "u_normal"} );
	a.m_links.push_back( {"linearDepth", "u_linearDepth"} );

	std::cout << "  diagnosting &v = " << &m_framebuffers[0] << ", &p = " << a.m_input.get() << '\n';
	a.m_input->bind();
	std::cout << "eb5 " << glGetError() << '\n';
	compositeRef->bind();
	std::cout << "eb6 " << glGetError() << '\n';

	lightingInputs.push_back( a );

	//The first stage.
	ShadingStage lighting( lightingInputs, compositeRef );
	lighting.m_attachments = {GL_COLOR_ATTACHMENT0};
	lighting.m_shader = "deferred_light";

	deferredStages.push_back( lighting );

	ShadingPipeline deferred ( dataInputRef, deferredStages );

	m_pipelines.insert( {"deferred", deferred} );

	std::vector<ngl::Vec4> screenQuadPoints = {
		ngl::Vec4( -1.0f, -1.0f, 0.0f, 1.0f ),
		ngl::Vec4( 1.0f, -1.0f, 0.0f, 1.0f ),
		ngl::Vec4( -1.0f, 1.0f, 0.0f, 1.0f ),
		ngl::Vec4( 1.0f, 1.0f, 0.0f, 1.0f )
	};
	std::vector<ngl::Vec2> screenQuadUVs = {
		ngl::Vec2( 0.0f, 0.0f ),
		ngl::Vec2( 1.0f, 0.0f ),
		ngl::Vec2( 1.0f, 1.0f ),
		ngl::Vec2( 0.0f, 1.0f )
	};
	m_screenQuadVAO = createVAO(screenQuadPoints, screenQuadUVs);
	ShadingPipeline::setScreenQuad( m_screenQuadVAO );
}

void Renderer::createShader(const std::string _name, const std::string _vert, const std::string _frag, const std::string _geo, const std::string _tessctrl, const std::string _tesseval)
{
	ngl::ShaderLib * slib = ngl::ShaderLib::instance();

	slib->createShaderProgram( _name );

	slib->attachShader(_vert, ngl::ShaderType::VERTEX);

	std::string vertshadersource = loadShaderToString( "shaders/" + _vert + ".glsl" );
	slib->loadShaderSourceFromString( _vert, vertshadersource );
	slib->compileShader(_vert);
	slib->attachShaderToProgram(_name, _vert);

	slib->attachShader(_frag, ngl::ShaderType::FRAGMENT);

	std::string fragshadersource = loadShaderToString( "shaders/" + _frag + ".glsl" );
	slib->loadShaderSourceFromString(_frag, fragshadersource );
	slib->compileShader(_frag);
	slib->attachShaderToProgram(_name, _frag);

	// add geometry shader if string given
	if(!_geo.empty())
	{
		std::string geoshadersource = loadShaderToString( "shaders/" + _vert + ".glsl" );
		slib->attachShader(_geo, ngl::ShaderType::GEOMETRY);
		slib->loadShaderSourceFromString(_geo, geoshadersource);
		slib->compileShader(_geo);
		slib->attachShaderToProgram(_name, _geo);
	}

	// add tesselation source, if present
	if(!_tessctrl.empty())
	{
		std::string tessctrlshadersource = loadShaderToString( "shaders/" + _vert + ".glsl" );
		slib->attachShader(_tessctrl, ngl::ShaderType::TESSCONTROL);
		slib->loadShaderSourceFromString(_tessctrl, tessctrlshadersource);
		slib->compileShader(_tessctrl);
		slib->attachShaderToProgram(_name, _tessctrl);
	}

	if(!_tesseval.empty())
	{
		std::string tessevalshadersource = loadShaderToString( "shaders/" + _vert + ".glsl" );
		slib->attachShader(_tesseval, ngl::ShaderType::TESSEVAL);
		slib->loadShaderSourceFromString(_tesseval, tessevalshadersource);
		slib->compileShader(_tesseval);
		slib->attachShaderToProgram(_name, _tesseval);
	}

	slib->linkProgramObject(_name);
}

std::string Renderer::loadShaderToString(const std::string &_path)
{
	std::vector<std::string> lines = File::getLinesFromFile( _path );

	std::string ret = "";
	for( auto &i : lines )
	{
		std::vector< std::string > frags = Utility::split( i, ' ' );
		if( frags.size() == 0 )
			continue;

		if( frags[0] == "#import" )
		{
			if( frags.size() > 1)
				ret += loadShaderToString( "shaders/" + frags[1] + ".glsl" );
			else
				Utility::errorExit( "Error! #import statement in " + _path + " with no following shader file!");
		}
		else
		{
			ret += i + '\n';
		}
	}
	return ret;
}

void Renderer::shader(const std::string &_shader)
{
	ngl::ShaderLib * slib = ngl::ShaderLib::instance();
	slib->use( _shader );
}

void Renderer::draw(const std::string &_mesh, const ngl::Vec3 &_pos, const ngl::Vec3 &_rot)
{
	m_transform.setPosition( _pos );
	m_transform.setRotation( _rot );

	ngl::Obj * mesh = s_assetStore.getModel( _mesh );
	if( mesh == nullptr )
		Utility::errorExit( "Error! Mesh " + _mesh + " does not exist!" );

	loadMatricesToShader();
	mesh->draw();
}

void Renderer::render()
{
	for(auto &i : m_pipelines)
		i.second.execute();
	/*glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	clear();*/

	for(auto &i : m_pipelines)
			i.second.dump();
}

void Renderer::shadingPipeline(const std::string &_pipe)
{
	std::cout << "Binding " << _pipe << " for input.\n";
	m_pipelines.at( _pipe ).bindInput();
}

void Renderer::clear()
{
	glClearColor(1.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::swap()
{
	SDL_GL_SwapWindow( m_window );
}

GLuint Renderer::createVAO(std::vector<ngl::Vec4> &_verts)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Generate a VBO
	GLuint vertBuffer = createBuffer4f( _verts );
	setBufferLocation( vertBuffer, 0, 4 );

	std::vector<ngl::Vec2> uvs;
	uvs.assign( _verts.size(), ngl::Vec2(0.0f, 0.0f) );
	GLuint UVBuffer = createBuffer2f( uvs );
	setBufferLocation( UVBuffer, 1, 2 );

	//Gen normals.
	std::vector<ngl::Vec3> normals;
	//Zero the elements
	normals.reserve( _verts.size());

	for(size_t i = 0; i < _verts.size(); i += 3)
	{
		ngl::Vec4 a = _verts[i + 1] - _verts[i];
		ngl::Vec4 b = _verts[i + 2] - _verts[i];
		ngl::Vec3 a3 = ngl::Vec3(a.m_x, a.m_y, a.m_z);
		ngl::Vec3 b3 = ngl::Vec3(b.m_x, b.m_y, b.m_z);
		ngl::Vec3 normal = a3.cross(b3);
		normal.normalize();
		//normal = -normal;
		for(int i = 0; i < 3; ++i)
			normals.push_back(normal);
	}

	GLuint normBuffer = createBuffer3f(normals);
	setBufferLocation( normBuffer, 2, 3 );

	return vao;
}

GLuint Renderer::createVAO(std::vector<ngl::Vec4> &_verts, std::vector<ngl::Vec2> &_uvs)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Generate a VBO
	GLuint vertBuffer = createBuffer4f( _verts );
	setBufferLocation( vertBuffer, 0, 4 );

	GLuint UVBuffer = createBuffer2f( _uvs );
	setBufferLocation( UVBuffer, 1, 2 );

	return vao;
}

GLuint Renderer::createVAO(std::vector<ngl::Vec4> &_verts, std::vector<ngl::Vec3> &_normals, std::vector<ngl::Vec2> &_uvs)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Generate a VBO
	GLuint vertBuffer = createBuffer4f( _verts );
	setBufferLocation( vertBuffer, 0, 4 );

	GLuint normBuffer = createBuffer3f( _normals );
	setBufferLocation( normBuffer, 2, 3 );

	GLuint UVBuffer = createBuffer2f( _uvs );
	setBufferLocation( UVBuffer, 1, 2 );

	return vao;
}

GLuint Renderer::createBuffer4f(std::vector<ngl::Vec4> _vec)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
							 sizeof(ngl::Vec4) * _vec.size(),
							 &_vec[0].m_x,
			GL_STATIC_DRAW
			);
	return buffer;
}

GLuint Renderer::createBuffer3f(std::vector<ngl::Vec3> _vec)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
							 sizeof(ngl::Vec3) * _vec.size(),
							 &_vec[0].m_x,
			GL_STATIC_DRAW
			);
	return buffer;
}

GLuint Renderer::createBuffer2f(std::vector<ngl::Vec2> _vec)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
							 sizeof(ngl::Vec2) * _vec.size(),
							 &_vec[0].m_x,
			GL_STATIC_DRAW
			);
	return buffer;
}

GLuint Renderer::createBuffer1f(std::vector<float> _vec)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
							 sizeof(float) * _vec.size(),
							 &_vec[0],
			GL_STATIC_DRAW
			);
	return buffer;
}

void Renderer::setBufferLocation(GLuint _buffer, int _index, int _size)
{
	glEnableVertexAttribArray(_index);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer);
	glVertexAttribPointer( _index, _size, GL_FLOAT, GL_FALSE, 0, 0 );
}

void Renderer::loadMatricesToShader(const ngl::Mat4 _M, const ngl::Mat4 _MVP)
{
	ngl::ShaderLib * slib = ngl::ShaderLib::instance();
	slib->setRegisteredUniform( "M", _M );
	slib->setRegisteredUniform( "MVP", _MVP );
}

void Renderer::loadMatricesToShader()
{
	ngl::ShaderLib * slib = ngl::ShaderLib::instance();
	ngl::Mat4 M = m_transform.getMatrix();
	ngl::Mat4 MVP = M * m_cam->getVP();

	slib->setRegisteredUniform( "M", M );
	slib->setRegisteredUniform( "MVP", MVP );
}
