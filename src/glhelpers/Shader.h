#pragma once

#include <epoxy/gl.h>

#include <string>

class GLShader
{
  public:
    GLShader(GLenum type);
    GLShader(const GLShader &) = delete;
    ~GLShader();

    bool compile( const std::string& source );
    bool isCompiled() const;
    GLenum getType() const;

    static const std::string& getLastError();
    
    const GLuint id;
};