#include <iostream>
#include <string>

#include "Framebuffer.hpp"
#include "Utility.hpp"

void Framebuffer::cleanup()
{
	glDeleteFramebuffers(1, &m_framebuffer);

	for(auto &tex : m_textures)
		glDeleteTextures( 1, &tex.second );
}

void Framebuffer::init(int _w, int _h)
{
	m_w = _w;
	m_h = _h;

	glGenFramebuffers(1, &m_framebuffer);
	bind();

	GLint numColAttachments = 0;
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &numColAttachments );
	m_maxColourTarget = numColAttachments + GL_COLOR_ATTACHMENT0;
}

void Framebuffer::activeColourAttachments()
{
	for(auto &buf : m_colourAttachments)
	{
		if(buf > m_maxColourTarget)
		{
			std::cerr << "Error! Attempting to use colour target " << std::to_string((int)buf) << "that is not available on this system (max is " << std::to_string((int)m_maxColourTarget) << ").\n";
			exit(EXIT_FAILURE);
		}
	}
	glDrawBuffers(m_colourAttachments.size(), &m_colourAttachments[0]);
}

void Framebuffer::activeColourAttachments(const std::vector<GLenum> &_bufs)
{
	for(auto &buf : _bufs)
		if(buf > m_maxColourTarget)
		{
			std::cerr << "Error! Attempting to use colour target " << std::to_string((int)buf) << "that is not available on this system (max is " << std::to_string((int)m_maxColourTarget) + ").\n";
			exit(EXIT_FAILURE);
		}

	for(auto &i : _bufs)
		glDrawBuffers( _bufs.size(), &_bufs[0] );
}

void Framebuffer::activeReadAttachment(const GLenum _buf)
{
	glReadBuffer(_buf);
}

void Framebuffer::addDepthAttachment(const std::string &_identifier)
{
	GLuint depth;
	glGenRenderbuffers(1, &depth);
	glBindRenderbuffer(GL_RENDERBUFFER, depth);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_w, m_h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	std::pair<std::string, GLuint> tex ( _identifier, depth );
	m_textures.insert( tex );
}

void Framebuffer::addTexture(const std::string &_identifier, GLuint _tex, GLenum _attachment)
{
	m_textures.insert( std::make_pair(_identifier, _tex) );

	glFramebufferTexture2D(GL_FRAMEBUFFER, _attachment, GL_TEXTURE_2D, m_textures[ _identifier ], 0);

	m_colourAttachments.push_back( _attachment );
}

void Framebuffer::addTexture(const std::string &_identifier, GLenum _format, GLenum _iformat , GLenum _attachment, GLint _type)
{
	//Create texture.
	std::pair<std::string, GLuint> tex ( _identifier, genTexture(m_w, m_h, _format, _iformat, _type) );
	m_textures.insert( tex );

	glFramebufferTexture2D(GL_FRAMEBUFFER, _attachment, GL_TEXTURE_2D, m_textures[ _identifier ], 0);

	m_colourAttachments.push_back( _attachment );
}

void Framebuffer::addTextureArray(
		const std::string &_identifier,
		GLenum _iformat,
		GLenum _attachment,
		size_t _layers
		)
{
	//Create texture.
	std::pair<std::string, GLuint> tex ( _identifier, genTextureArray(m_w, m_h, _layers, _iformat ) );
	m_textures.insert( tex );

	glFramebufferTexture2D(GL_FRAMEBUFFER, _attachment, GL_TEXTURE_2D, m_textures[ _identifier ], 0);

	m_colourAttachments.push_back( _attachment );
}

void Framebuffer::bind( GLenum type )
{
	glBindFramebuffer(type, m_framebuffer);
}

void Framebuffer::bindTexture(const GLint _shaderID, const std::string &_tex, const char *_uniform, int _target)
{
	GLint loc = glGetUniformLocation(_shaderID, _uniform);

	if(loc == -1)
		Utility::warning( "Uh oh! Invalid uniform location in Framebuffer::bindTexture!! " + std::string(_uniform) + '\n' );

	glUniform1i(loc, _target);

	glActiveTexture(GL_TEXTURE0 + _target);
	glBindTexture(GL_TEXTURE_2D, m_textures.at( _tex ) );
}

bool Framebuffer::checkComplete()
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::clear()
{
	//activeColourAttachments( );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLuint Framebuffer::genTexture(int _width, int _height, GLenum _format, GLint _internalFormat, GLenum _type)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, _width, _height, 0, _format, _type, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return tex;
}

GLuint Framebuffer::genTextureArray(
		int _width,
		int _height,
		int _layers,
		GLint _internalFormat)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

	glTexStorage3D(GL_TEXTURE_2D, _layers, _internalFormat, _width, _height, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return tex;
}

void Framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
