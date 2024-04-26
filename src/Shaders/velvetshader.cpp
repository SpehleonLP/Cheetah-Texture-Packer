#include "velvetshader.h"
#include "defaultvaos.h"
#include "src/widgets/glviewwidget.h"

#define SHADER(k) "#version 150\n" #k

#define TEX_SIZE_X 144
#define TEX_SIZE_Y 150
#define TEX_SIZE2 (TEX_SIZE_X * TEX_SIZE_Y)

static const char * kVert();
static const char * kFrag();

VelvetShader VelvetShader::Shader;

void VelvetShader::construct(GLViewWidget* gl)
{
    compile(gl, kVert(), GL_VERTEX_SHADER);
    compile(gl, kFrag(), GL_FRAGMENT_SHADER);
    attribute(gl, 0, "a_vertex");
    attribute(gl, 2, "a_uv");
    link(gl);
	uniform(gl, u_texture, "u_texture");
	uniform(gl, u_color  , "u_color"  );
	uniform(gl, u_texSize, "u_texSize"  );

	glDefaultVAOs::AddRef();

	std::unique_ptr<uint8_t[]> buffer(new uint8_t[TEX_SIZE2]);

	for(uint32_t i = 0; i < TEX_SIZE2; ++i)
		buffer[i] = rand();

	_gl glGenTextures(1, &texture);
	_gl glBindTexture(GL_TEXTURE_2D, texture);

	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	_gl glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	_gl glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RED,
			TEX_SIZE_X,
			TEX_SIZE_Y,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			&buffer[0]);
}

void VelvetShader::destruct(GLViewWidget* gl)
{

	_gl glDeleteTextures(1, &texture);
	glDefaultVAOs::Release(gl);

	texture = 0;
}

void VelvetShader::bind(GLViewWidget* gl, glm::vec4 color)
{


    if(bindShader(gl))
	{
        _gl glDisable(GL_DEPTH_TEST);
        _gl glDepthMask(GL_FALSE);
        _gl glDisable(GL_BLEND);
        _gl glDisable(GL_CULL_FACE);

		_gl glActiveTexture(GL_TEXTURE10);
		_gl glBindTexture(GL_TEXTURE_2D, texture);
		_gl glActiveTexture(GL_TEXTURE0);

		_gl glUniform1i(u_texture, 10);
	}

	_gl glUniform4fv(u_color, 1, &color[0]);
	_gl glUniform2i(u_texSize, TEX_SIZE_X, TEX_SIZE_Y);

}

static const char * kVert()
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

		in vec3 a_vertex;
		in vec2 a_uv;

		out vec2 v_uv;

		void main()
		{
	//		gl_Position = u_projection * (u_modelview * vec4(a_vertex, 1.0));
			gl_Position = vec4(a_vertex, 1.0);
			v_uv        = a_uv;
		});
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
		uniform ivec2     u_texSize;

		in vec2 v_uv;
		in vec4 gl_FragCoord;

		out vec4 frag_color;

		void main()
		{
			vec2  uv     = gl_FragCoord.xy / u_texSize;
			vec4  color  = texture(u_texture, uv);
			color = vec4(color.r, color.r, color.r, 1);
			frag_color = vec4(mix(color.rgb, u_color.rgb, u_color.a), 1);
		});
}

