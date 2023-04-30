#include "Utils.h"

#include <SDL_opengles2.h>

#include <iostream>
#include <sstream>

using std::clog;
using std::endl;

const char *error_string(GLenum e)
{
    switch (e)
    {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "unknown OpenGL error";
    }
}

bool check_gl_error(const char *file, int line)
{
    bool has_error = false;

    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR)
    {
        clog << "OpenGL error fetched at " << file << "#" << line << ": " << error_string(err) << endl;
        has_error = true;
    }
    return has_error;
}

GLuint get_binding(GLenum what)
{
    GLint result = -1;
    glGetIntegerv(what, &result);
    return GLuint(result);
}