#include "transparencyshader.h"
#include "defaultvaos.h"
#include "src/widgets/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k

static const char * kFrag();

TransparencyShader TransparencyShader::Shader;

void TransparencyShader::construct(GLViewWidget* gl)
{
    compile(gl, SheetVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
	SheetLink(gl);

	glDefaultVAOs::AddRef();
}

void TransparencyShader::destruct(GLViewWidget* gl)
{
	_gl glAssert();
	glDefaultVAOs::Release(gl);
}

void TransparencyShader::bind(GLViewWidget* gl, Material *)
{
	GL_ASSERT;

    if(bindShader(gl))
	{
        _gl glDisable(GL_DEPTH_TEST);
        _gl glDepthMask(GL_FALSE);
        _gl glDisable(GL_BLEND);
        _gl glDisable(GL_CULL_FACE);
	}

	_gl glUniform1i(u_bufferTexture, 10);
	_gl glUniform1f(u_layer , 0);

	_gl glActiveTexture(GL_TEXTURE10);
	_gl glBindTexture(GL_TEXTURE_BUFFER, 0);

    GL_ASSERT;
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

		in vec4 gl_FragCoord;

		out vec4 frag_color;

		void main()
		{
			ivec2 coords = ivec2(gl_FragCoord);
			int tile = (int(coords.x / 10) & 0x01) ^ (int(coords.y / 10) & 0x01);
			float color = (125 + 100 * tile) / 256.f;
			frag_color = vec4(vec3(color), 1.0);
		});
}

