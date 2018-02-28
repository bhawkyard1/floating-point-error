#include <array>

#include <ngl/NGLInit.h>
#include <ngl/ShaderLib.h>

#include "Renderer.hpp"
#include "File.hpp"
#include "Utility.hpp"

#ifdef _WIN32
#include <ciso646>
#endif

//TODO: delete
#include <NGL/NGLStream.h>

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

	//Enables and disabled.
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

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

	std::cout << "Creating screen squad mesh...\n";
	std::vector<ngl::Vec4> screenQuadPoints = {
		ngl::Vec4( -1.0f, -1.0f, 0.0f, 1.0f ),
		ngl::Vec4( 1.0f, -1.0f, 0.0f, 1.0f ),
		ngl::Vec4( -1.0f, 1.0f, 0.0f, 1.0f ),
		ngl::Vec4( 1.0f, 1.0f, 0.0f, 1.0f )
	};
	std::vector<ngl::Vec2> screenQuadUVs = {
		ngl::Vec2( 0.0f, 0.0f ),
		ngl::Vec2( 1.0f, 0.0f ),
		ngl::Vec2( 0.0f, 1.0f ),
		ngl::Vec2( 1.0f, 1.0f )
	};
	m_screenQuadVAO = createVAO(screenQuadPoints, screenQuadUVs);
	ShadingPipeline::setScreenQuad( m_screenQuadVAO );

	std::cout << "Generating light data...\n";
	//Setup lighting buffers
	m_renderLights.assign( m_maxLights, RenderLight() );
	glGenBuffers( 1, &m_lightBuffer );
	glBindBuffer( GL_UNIFORM_BUFFER, m_lightBuffer );
	std::cout << "p1\n";
	glBufferData( GL_UNIFORM_BUFFER, sizeof( RenderLight ) * m_maxLights, &m_renderLights[0], GL_DYNAMIC_DRAW );
	std::cout << "p2\n";
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );

	int NUM_CASCADES_WHERE = 3;
	m_shadowMatrices.assign( m_maxLights * NUM_CASCADES_WHERE, ngl::Mat4() );

	std::cout << "Generating deferred shading pipeline...\n";
	//Set up deferred shading pipeline.
	//Framebuffers
	Framebuffer gBuffer;
	gBuffer.init(
				m_dimensions.m_x,
				m_dimensions.m_y
				);
	gBuffer.addTexture( "diffuse", GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT0 );
	gBuffer.addTexture( "position", GL_RGBA, GL_RGBA16F, GL_COLOR_ATTACHMENT1 );
	gBuffer.addTexture( "normal", GL_RGBA, GL_RGBA16F, GL_COLOR_ATTACHMENT2 );
	gBuffer.addTexture( "linearDepth", GL_RED, GL_RED, GL_COLOR_ATTACHMENT3 );
	gBuffer.addDepthAttachment( "depth" );
	if(!gBuffer.checkComplete())
		Utility::errorExit("Uh oh! Framebuffer incomplete! Error code " + glGetError() + '\n');
	gBuffer.unbind();

	m_framebuffers.push_back( gBuffer );
	MemRef< Framebuffer > dataInputRef ( m_framebuffers.backID() );

	Framebuffer compositeBuffer;
	compositeBuffer.init(
				m_dimensions.m_x,
				m_dimensions.m_y
				);
	compositeBuffer.addTexture( "solid", GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT0 );
	compositeBuffer.addTexture( "nonsolid", GL_RGBA, GL_RGBA, GL_COLOR_ATTACHMENT1 );
	compositeBuffer.addDepthAttachment( "depth" );
	if(!compositeBuffer.checkComplete())
		Utility::errorExit("Uh oh! Framebuffer incomplete! Error code " + glGetError() + '\n');
	compositeBuffer.unbind();

	m_framebuffers.push_back( compositeBuffer );
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

	lightingInputs.push_back( a );

	//The first stage.
	ShadingStage lighting( lightingInputs, compositeRef );
	lighting.m_attachments = {GL_COLOR_ATTACHMENT0};
	lighting.m_shader = "deferred_light";
	lighting.m_dataBuffers.push_back(
				UniformBuffer( m_lightBuffer, "lbuf" )
				);

	deferredStages.push_back( lighting );

	ShadingPipeline deferred ( dataInputRef, deferredStages );

	m_pipelines.insert( {"deferred", deferred} );

	//Generate the shadowbuffer. This is a single framebuffer with an array texture.
	//As opposed to my initial idea, vector of buffers
	m_shadowBuffer.init(
				m_dimensions.m_x,
				m_dimensions.m_y
				);
	m_shadowBuffer.addTextureArray( "depths", GL_RED, GL_COLOR_ATTACHMENT0, 3 );

	std::cout << "Renderer construction completed.\n";
}

Renderer::~Renderer()
{
	for( auto &buf : m_framebuffers.m_objects )
		buf.cleanup();

	m_shadowBuffer.cleanup();
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
	std::vector<std::string> spl = Utility::split(fragshadersource, '\n');
	for( size_t i = 0; i < spl.size(); ++i )
	{
		std::cout << i << " " << spl[i] << '\n';
	}

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

void Renderer::update()
{
	std::cout << "Renderer::update 1\n";
	m_renderLights.clear();
	std::cout << "Renderer::update 1.1\n";
	for( auto &light : m_lights->m_objects )
		m_renderLights.push_back( light.getRenderData() );
	std::cout << "Renderer::update 1.2\n";
	if( m_renderLights.size() > 0 )
	{
		std::cout << "Renderer::update 1.3\n";
		glBindBuffer(GL_UNIFORM_BUFFER, m_lightBuffer);
		std::cout << "Renderer::update 1.31\n";
		GLvoid * dat = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		std::cout << "Renderer::update 1.32 dat ptr is null? " << (dat==nullptr) << "\n";
		memcpy(dat, &m_renderLights[0], sizeof(RenderLight) * std::min( m_renderLights.size(), static_cast<size_t>(m_maxLights)));
		std::cout << "Renderer::update 1.33\n";
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		std::cout << "Renderer::update 1.3\n";
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		std::cout << "Renderer::update 1.4\n";
	}
	std::cout << "Renderer::update 2\n";
	generateShadowData({0.01f, 16.0f, 64.0f, 1024.0f});
	std::cout << "Renderer::update 3\n";
}

void Renderer::draw(
		const std::string &_mesh,
		const ngl::Vec3 &_pos,
		const ngl::Vec3 &_rot,
		const bool _shadows
		)
{
	//Draw mesh into active buffer.
	m_transform.reset();
	m_transform.setPosition( _pos );
	m_transform.setRotation( _rot );

	ngl::Obj * mesh = s_assetStore.getModel( _mesh );
	if( mesh == nullptr )
		Utility::errorExit( "Error! Mesh " + _mesh + " does not exist!" );

	loadMatricesToShader();
	mesh->draw();

	if( !_shadows )
		return;

	//Draw mesh into shadow buffers.
	m_transform.reset();

	ngl::ShaderLib * slib = ngl::ShaderLib::instance();
	for( auto &data : m_shadowMatrices )
	{
		m_shadowBuffer.bind();
		for( int i = 0; i < m_shadowCascades; ++i )
		{
			/*glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						GL_DEPTH_ATTACHMENT,
						GL_TEXTURE_2D,
						data.m_framebuffer.get( "depth" + std::to_string( i ) ),
						0
						);*/
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			slib->use( "deferred_shadowDepth" );
			loadMatricesToShader( ngl::Mat4(), data );
			mesh->draw();
		}
	}
}

void Renderer::render()
{
	for(auto &i : m_pipelines)
		i.second.execute();

	for(auto &i : m_pipelines)
		i.second.dump();
}

void Renderer::shadingPipeline(const std::string &_pipe)
{
	m_pipelines.at( _pipe ).bindInput();
}

void Renderer::clear()
{
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

void Renderer::generateShadowData(const std::vector<float> &_segDepths)
{
	//If we proceed with _segDepths being zero, the program will later crash.
	if( _segDepths.size() == 0 )
		return;

	std::cout << "Renderer::generateShadowData 1\n";
	//Get the coordinates of each camera cascade, in world space.
	using Cascade = std::array< ngl::Vec4, 8 >;
	std::vector< Cascade > cascades;
	for( size_t i = 0 ; i < _segDepths.size() - 1; ++i )
	{
		Cascade c = m_cam->calculateCascade(
					_segDepths[ i ],
					_segDepths[ i + 1 ]
				);
		cascades.push_back( c );
	}
	std::cout << "Renderer::generateShadowData 2\n";
	//Cast the vertices of the camera frustrum into light space.
	int shadowDataIndex = 0;
	for( auto &light : m_lights->m_objects )
	{
		if( !light.m_shadowCasting )
			continue;

		/*
		//Create a new shadow data if we are out of spares.
		if( shadowDataIndex >= m_shadowData.size() )
		{
			Framebuffer f;
			int shadowResolution = 1024;
			f.init( shadowResolution, shadowResolution );
			for( size_t i = 0; i < m_shadowCascades; ++ i )
				f.addTexture( "depth" + std::to_string( i ),
											GL_DEPTH_COMPONENT,
											GL_DEPTH_COMPONENT,
											GL_DEPTH_ATTACHMENT );
			if( !f.checkComplete() )
				Utility::errorExit( "Error! Shadow framebuffer could not be created! Error " + std::to_string( glGetError() ) );
			f.unbind();
			ShadowData sd;
			sd.m_framebuffer = f;
			sd.m_matrices = std::vector< ngl::Mat4 >();
			m_shadowData.push_back( sd );
		}*/

		ngl::Vec3 rot = light.getRot();
		ngl::Transformation t;
		t.setRotation( rot );
		ngl::Mat4 rotMat = t.getMatrix();

		std::vector< Cascade > lightSpaceCascades;
		for( auto c : cascades )
		{
			Cascade cL;
			for( size_t i = 0; i < c.size(); ++i )
			{
				cL[i] = c[i] * rotMat;
			}
			lightSpaceCascades.push_back( cL );
		}

		std::cout << "Renderer::generateShadowData 3\n";
		using Bounds = std::pair< ngl::Vec3, ngl::Vec3 >;
		for( auto cL : lightSpaceCascades )
		{
			Bounds bounds = Utility::enclose( cL );
			ngl::Vec3 pos = -(bounds.first + bounds.second) / 2.0f;
			ngl::Vec3 dim = (bounds.second - bounds.first) / 2.0f;
			ngl::Mat4 boundsTrans = ngl::Mat4();
			boundsTrans.translate( pos.m_x, pos.m_y, pos.m_z );

			ngl::Mat4 project = ngl::ortho(
						-dim.m_x, dim.m_x,
						-dim.m_y, dim.m_y,
						-dim.m_z, dim.m_z
						);
			ngl::Mat4 view = ngl::lookAt(
						ngl::Vec3(),
						rotMat.getForwardVector(),
						ngl::Vec3( 0.0f, 1.0f, 0.0f )
						);

			m_shadowMatrices[ shadowDataIndex ] = project * view;
		}
		std::cout << "Renderer::generateShadowData 3.1\n";
	}
	std::cout << "Renderer::generateShadowData 4\n";
	shadowDataIndex++;
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

//TODO delete
void Renderer::debug()
{
	ngl::ShaderLib * slib = ngl::ShaderLib::instance();
	slib->use("utility_colour");

	auto ref = &m_framebuffers[0];

	ref->bind();
	ref->activeColourAttachments( );
	//glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glClearColor(1.0f,0.0f,1.0f,1.0f);
	clear();
	draw( "sphere" );
	//swap();
	//return;

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glClearColor(1.0f,0.0f,1.0f,1.0f);
	clear();
	glBindVertexArray( m_screenQuadVAO );

	slib->use("utility_texCopy");
	m_transform.reset();
	loadMatricesToShader();

	GLuint id = slib->getProgramID("utility_texCopy");
	ref->bindTexture( id, "diffuse", "u_tex", 0 );
	glDrawArraysEXT( GL_TRIANGLE_STRIP, 0, 4 );

	glBindVertexArray( 0 );
	swap();
}
