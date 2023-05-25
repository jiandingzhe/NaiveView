#include "Texture.h"
#include "Utils.h"

#include <cassert>
#include <cstdlib>

GLTexture2D::BindScope::BindScope(GLTexture2D &obj) : obj(obj)
{
    glBindTexture(obj.type, obj.id);
}

GLTexture2D::BindScope::~BindScope()
{
#ifndef NDEBUG
    GLuint curr_tex = 0;
    switch (obj.type)
    {
    case GL_TEXTURE_2D:
        curr_tex = get_binding(GL_TEXTURE_BINDING_2D);
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        curr_tex = get_binding(GL_TEXTURE_BINDING_EXTERNAL_OES);
        break;
    default:
        abort();
    }
    assert(curr_tex == obj.id);
#endif
}

void GLTexture2D::BindScope::setMono8Image(int w, int h, const void *data)
{
    glTexImage2D(obj.type, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    CHECK_GL;
}

void GLTexture2D::BindScope::setRGBImage(int w, int h, const void *data)
{
    glTexImage2D(obj.type, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    CHECK_GL;
}

static GLuint gen_texture()
{
    GLuint re = -1;
    glGenTextures(1, &re);
    assert(re >= 0);
    return re;
}

GLTexture2D::GLTexture2D(GLenum type, bool mag_linear, bool min_linear, bool mipmap) : id(gen_texture()), type(type)
{
    BindScope scope(*this);
    if (min_linear)
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    else
        glTexParameteri(type, GL_TEXTURE_MIN_FILTER, mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);

    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, mag_linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECK_GL;
}

GLTexture2D::~GLTexture2D()
{
    glDeleteTextures(1, &id);
}