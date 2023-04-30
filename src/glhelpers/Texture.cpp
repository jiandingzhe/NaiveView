#include "Texture.h"
#include "Utils.h"

#include <GLES2/gl2.h>
#include <cassert>

GLTexture2D::BindScope::BindScope(GLTexture2D &obj) : obj(obj)
{
    glBindTexture(GL_TEXTURE_2D, obj.id);
}

GLTexture2D::BindScope::~BindScope()
{
#ifndef NDEBUG
    GLuint curr_tex = get_binding(GL_ELEMENT_ARRAY_BUFFER_BINDING);
    assert(curr_tex == obj.id);
#endif
}

void GLTexture2D::BindScope::setMono8Image(int w, int h, const void *data)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    CHECK_GL;
}

static GLuint gen_texture()
{
    GLuint re = -1;
    glGenTextures(1, &re);
    assert(re >= 0);
    return re;
}

GLTexture2D::GLTexture2D(bool mag_linear, bool min_linear, bool mipmap) : id(gen_texture())
{
    BindScope scope(*this);
    if (min_linear)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECK_GL;
}

GLTexture2D::~GLTexture2D()
{
    glDeleteTextures(1, &id);
}