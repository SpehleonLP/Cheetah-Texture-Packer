#include "glprogram.h"
#include "src/widgets/glviewwidget.h"
#include <cstdio>
#include <exception>
#include <vector>
#include <cassert>
#include <QMessageBox>
#include <QGuiApplication>

glProgram::glProgram() :
    m_refCount(0L),
    m_program(0L)
{
    shaders.fill(0);
}

glProgram::~glProgram()
{
}


void glProgram::AddRef()
{
    lock_guard_t lock(m_mutex);
    ++m_refCount;
}

void glProgram::Release(OpenGL* gl)
{
    lock_guard_t lock(m_mutex);
    if( --m_refCount == 0 )
    {
        onDestruct(gl);
    }

    assert(m_refCount >= 0);
}

void glProgram::onDestruct(OpenGL * gl)
{
	

    if(m_program)
    {
		destruct(gl);

        for(size_t i = 0; i < shaders.size(); ++i)
        {
            _gl glDeleteShader(shaders[i]); 
            shaders[i] = 0L;
        }

        _gl glDeleteProgram(m_program); 
        m_program = 0L;
    }
}

void glProgram::printLog(OpenGL * gl)
{
    GLint length;
    _gl glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length); 

    std::vector<char> log(length, 0);
    _gl glGetProgramInfoLog(m_program, length, &length, log.data()); 
    fprintf(stderr, "%s", log.data());
    fflush(stderr);
}

int glProgram::getShaderIndex(GLuint type)
{
    switch(type)
    {
    default: throw std::logic_error("unknown shader type");
    case GL_COMPUTE_SHADER:			return 0;
    case GL_VERTEX_SHADER:			return 1;
    case GL_TESS_CONTROL_SHADER:	return 2;
    case GL_TESS_EVALUATION_SHADER:	return 3;
    case GL_GEOMETRY_SHADER:		return 4;
    case GL_FRAGMENT_SHADER:		return 5;
    }
}

void glProgram::compile(OpenGL * gl, const char * text, GLuint type)
{
    int i = getShaderIndex(type);

    if(shaders[i])
    {
        throw std::logic_error("tried to add shader type which already exists");
    }

    shaders[i] = compileShader(gl, text, type);

    if(shaders[i] == 0)
    {
        throw std::logic_error("failed to add shader");
    }

    _gl glAttachShader(m_program, shaders[i]); 
}


bool glProgram::tryLoad(OpenGL * gl, const char * filename, GLuint type)
{
    int i = getShaderIndex(type);

    if(shaders[i])
    {
        throw std::logic_error("tried to add shader type which already exists");
    }

    shaders[i] = loadShader(gl, filename, type);

    if(shaders[i] == 0)
    {
        return false;
    }

    _gl glAttachShader(m_program, shaders[i]); 
    return true;
}


void glProgram::load(OpenGL * gl, const char * filename, GLuint type)
{
    int i = getShaderIndex(type);

    if(shaders[i])
    {
        throw std::logic_error("tried to add shader type which already exists");
    }

    shaders[i] = loadShader(gl, filename, type);

    if(shaders[i] == 0)
    {
        throw std::logic_error("failed to add shader");
    }

    _gl glAttachShader(m_program, shaders[i]); 
}

void glProgram::link(OpenGL * gl)
{
    _gl glLinkProgram(m_program); 

    GLint status;
    _gl glGetProgramiv(m_program, GL_LINK_STATUS, &status); 

    if(status == GL_FALSE)
    {
        printLog(gl);
        throw std::runtime_error("link failed");
    }
}

bool glProgram::validate(OpenGL * gl)
{
    _gl glValidateProgram(m_program); 

    GLint status;
    _gl glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status); 

    if(status == GL_FALSE)
    {
        printLog(gl);
    }

    return status == GL_TRUE;
}

void glProgram::attribute(OpenGL * gl, GLuint index, const char * name)
{
    _gl glBindAttribLocation(m_program, index, name); 
}

bool glProgram::uniformBlock(OpenGL * gl, uint32_t binding, const char * name)
{
    GLuint block = _gl glGetUniformBlockIndex(m_program, name);
    if(block == GL_INVALID_INDEX)
        return false;

    _gl glUniformBlockBinding(m_program, block, binding);

    return true;
}

void glProgram::uniform(OpenGL * gl, GLint & index, const char * name)
{
    index = _gl glGetUniformLocation(m_program, name); 
}

bool glProgram::bindShader(OpenGL * gl)
{
    if(!m_program)
    {
        m_program = _gl glCreateProgram(); 
        construct(gl);
    }

    _gl glUseProgram(m_program); 
    return true;
}

GLuint glProgram::compileShader(OpenGL * gl, const char * text, GLuint shader_type)
{
    GLuint shader = 0;
    GLint length = 0;
    std::vector<char> error;

    shader = _gl glCreateShader(shader_type); 

    if(!shader)
    {
        fprintf(stderr, "unable to allocate glsl shader");
        return 0;
    }

    _gl glShaderSource(shader, 1, &text, 0L); 
    _gl glCompileShader(shader); 

    GLint success = 0;
    _gl glGetShaderiv(shader, GL_COMPILE_STATUS, &success); 

    if(success != GL_FALSE)
    {
        return shader;
    }

    length = 0;
    _gl glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length); 

    error.resize(length, 0);
    _gl glGetShaderInfoLog(shader, length, &length, error.data()); 

    fprintf(stderr, "%s", error.data());
    _gl glDeleteShader(shader); 
    return 0;
}

GLuint glProgram::loadShader(OpenGL * gl, const char * filename, GLuint shader_type)
{
    GLuint shader;
    GLint length;
    std::vector<char> text;
    char * text_ptr;

    FILE * file = fopen(filename, "r");

    if(!file)
    {
        fprintf(stderr, "unable to open glsl shader '%s'", filename);
        return 0;
    }

    shader = _gl glCreateShader(shader_type); 

    if(!shader)
    {
        fclose(file);
        fprintf(stderr, "unable to allocate glsl shader");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    text.resize(length, 0);
    fread(text.data(), 1, text.size(), file);
    fclose(file);

    text_ptr = text.data();
    _gl glShaderSource(shader, 1, &text_ptr, &length); 
    _gl glCompileShader(shader); 

    GLint success = 0;
    _gl glGetShaderiv(shader, GL_COMPILE_STATUS, &success); 

    if(success != GL_FALSE)
    {
        return shader;
    }

    length = 0;
    _gl glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length); 
    text.resize(length, 0);
    _gl glGetShaderInfoLog(shader, length, &length, text.data()); 
    _gl glDeleteShader(shader); 

    QMessageBox::information(nullptr, QGuiApplication::applicationDisplayName(), &text[0]);

    return 0;
}
