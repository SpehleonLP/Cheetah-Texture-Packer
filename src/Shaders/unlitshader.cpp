#include "unlitshader.h"
#include "defaultvaos.h"
#include <glm/vec4.hpp>
#include "src/widgets/glviewwidget.h"

static const char * kFrag();

UnlitShader UnlitShader::Shader;

void UnlitShader::construct(GLViewWidget* gl)
{
    compile(gl, SpriteVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
	SpriteLink(gl);

	uniform(gl, u_texture, "u_texture");
	uniform(gl, u_color,   "u_color");
	uniform(gl, u_useColor,"u_useColor");


	glDefaultVAOs::AddRef();
}

void UnlitShader::destruct(GLViewWidget* gl)
{
	_gl glAssert();
	glDefaultVAOs::Release(gl);
}

void UnlitShader::bind(GLViewWidget* gl, Material * )
{
    if(bindShader(gl))
	{
		 _gl glEnable(GL_DEPTH_TEST);
		 _gl glDepthMask(GL_TRUE);

		_gl glEnable(GL_BLEND);
		_gl glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		_gl glDisable(GL_CULL_FACE);
	}

	_gl glActiveTexture(GL_TEXTURE10);
	_gl glBindTexture(GL_TEXTURE_BUFFER, 0);

	_gl glActiveTexture(GL_TEXTURE0);
	_gl glBindTexture(GL_TEXTURE_BUFFER, 0);

	_gl glUniform1i(u_texture, 0);
	_gl glUniform1i(u_bufferTexture, 10);
	_gl glUniform1f(u_layer , 0);

	_gl glUniform1f(u_useColor , 0);
	_gl glUniform4f(u_color, 0, 0, 0, 0);



    _gl glAssert();
}

void UnlitShader::bindTexture(GLViewWidget* gl, uint32_t texture)
{
	_gl glBindTexture(GL_TEXTURE_2D, texture);
}

void UnlitShader::bindColor(GLViewWidget* gl, glm::vec4 color)
{
	_gl glUniform4fv(u_color, 1, &color[0]);
	_gl glUniform1f(u_useColor, 1.f);
}

void UnlitShader::clearColor(GLViewWidget* gl)
{
	_gl glUniform1f(u_useColor, 0.f);
}

static const char * kFrag()
{
	return SHADER(
		layout(std140) uniform Matrices
		{
			mat4  u_projection;
			mat4  u_modelview;
			ivec4 u_screenSize;
			vec4 u_cursorPos;
			float u_time;
		};

		uniform sampler2D u_texture;
		uniform vec4      u_color;
		uniform float     u_useColor;

		in vec3 v_position;
		in vec4 v_texCoord0;
		in vec4 v_texCoord1;

		out vec4 frag_color;

		void main()
		{
			vec4 color = texture(u_texture, v_texCoord0.xy);
			frag_color = color;
		});
}

