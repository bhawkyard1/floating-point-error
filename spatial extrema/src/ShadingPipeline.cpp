#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>

#include "ShadingPipeline.hpp"
#include "Utility.hpp"

GLuint ShadingPipeline::s_screenQuad;

ShadingPipeline::ShadingPipeline(const MemRef<Framebuffer> &_inputBuffer, const std::vector<ShadingStage> &_stages)
	:
		m_inputBuffer( _inputBuffer ),
		m_stages( _stages )
{
}

void ShadingPipeline::execute()
{
	ngl::ShaderLib * slib = ngl::ShaderLib::instance();
	slib->setRegisteredUniform( "M", ngl::Mat4() );
	glBindVertexArray( s_screenQuad );
	for(auto &stage : m_stages)
	{
		//Set the shader, and get its ID for later use.
		slib->use( stage.m_shader );
		slib->setRegisteredUniform( "M", ngl::Mat4() );
		GLint id = slib->getProgramID( stage.m_shader );

		//Cycle through the stage inputs, wire up textures to uniforms.
		for(auto &input : stage.m_inputs )
		{
			//Wire up each individual link.
			int cnt = 0;
			for( auto &link : input.m_links )
			{
				input.m_input->bindTexture( id, link.first, link.second.c_str(), cnt );
				cnt++;
			}
		}

		//Bind the output buffer.
		stage.m_output->bind();
		stage.m_output->activeColourAttachments( stage.m_attachments );

		//Do I need to clear?
		glClearColor(0.0,0.0,0.0,1.0);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//Draw screen quad.
		glDrawArraysEXT( GL_TRIANGLE_STRIP, 0, 4 );
	}
	glBindVertexArray( 0 );
}

void ShadingPipeline::bindInput( )
{
	m_inputBuffer->bind();
	m_inputBuffer->activeColourAttachments();
}

void ShadingPipeline::dump()
{
	ngl::ShaderLib * slib = ngl::ShaderLib::instance();
	slib->use( "utility_texCopy" );
	ngl::Transformation tr;
	//tr.setPosition(2.0,0.0,0.0);
	slib->setRegisteredUniform( "M", tr.getMatrix() );

	GLint id = slib->getProgramID( "utility_texCopy" );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	std::vector<GLenum> bufs = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(bufs.size(), &bufs[0]);

	MemRef<Framebuffer> end( m_stages.back().m_output );
	end->bindTexture( id, "solid", "u_tex", 0 );

	glBindVertexArray( s_screenQuad );

	glClearColor(0.0,0.0,0.0,1.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glDrawArraysEXT( GL_TRIANGLE_STRIP, 0, 4 );

	glBindVertexArray( 0 );
}
