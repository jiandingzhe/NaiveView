#include "Shader.h"

#include "Utils.h"
#include <GLES2/gl2.h>

#include <iostream>

GLShader::GLShader(GLenum type) : id(glCreateShader(type))
{
    CHECK_GL;
}

GLShader::~GLShader()
{
    glDeleteShader(id);
    CHECK_GL;
}

static std::string &_last_error_()
{
    static std::string error;
    return error;
}

bool GLShader::compile(const std::string &source)
{
    assert(id != 0);

    const char *source_ptr = source.c_str();
    int source_bytes = source.length();

    glShaderSource(id, 1, &source_ptr, &source_bytes);
    glCompileShader(id);

    GLint status = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLchar infoLog[4096] = {};
        GLsizei infoLogLength = 0;
        glGetShaderInfoLog(id, sizeof(infoLog), &infoLogLength, infoLog);
        _last_error_() = std::string(infoLog);
        assert(false);
        return false;
    }

    return true;
}

bool GLShader::isCompiled() const
{
    GLint type;
    glGetShaderiv(id, GL_COMPILE_STATUS, &type);
    return type == GL_TRUE;
}

GLenum GLShader::getType() const
{
    GLint type;
    glGetShaderiv(id, GL_SHADER_TYPE, &type);
    return type;
}

const std::string &GLShader::getLastError()
{
    return _last_error_();
}