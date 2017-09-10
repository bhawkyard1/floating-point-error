#ifndef SHADING_PIPELINE_HPP
#define SHADING_PIPELINE_HPP

#include <vector>

#include "Framebuffer.hpp"
#include "MemRef.hpp"

struct ShadingInput
{
	ShadingInput( const MemRef< Framebuffer > _input ) :
		m_input( _input )
	{
	}

	MemRef< Framebuffer > m_input;
	std::vector< std::pair< std::string, std::string > > m_links;
};

struct ShadingStage
{
	ShadingStage( const std::vector< ShadingInput > &_inputs, const MemRef< Framebuffer > &_output ) :
	m_inputs(_inputs),
	m_output(_output)
	{}

	//The framebuffers we are sourcing from.
	std::vector< ShadingInput > m_inputs;

	//The attachments in the output we will write to.
	std::vector< GLenum > m_attachments;
	//The shader we will use.
	std::string m_shader;
	//The output buffer.
	MemRef< Framebuffer > m_output;
};

class ShadingPipeline
{
public:
	ShadingPipeline(const MemRef< Framebuffer > &_inputBuffer, const std::vector< ShadingStage > &_stages);
	void execute();
	void dump();
	void bindInput();
	void setInput( MemRef< Framebuffer > &_buffer ) {m_inputBuffer = _buffer;}
	static void setScreenQuad( GLuint _screenQuad ) {s_screenQuad = _screenQuad;}
private:
	MemRef< Framebuffer > m_inputBuffer;
	std::vector< ShadingStage > m_stages;
	static GLuint s_screenQuad;
};

#endif
