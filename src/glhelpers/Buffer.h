#pragma once

#include <GLES2/gl2.h>

class GLBuffer
{
  public:
    class BindScope
    {
        BindScope(GLBuffer &);
        ~BindScope();
        void setSize(unsigned numBytes, GLenum usage = GL_STATIC_DRAW);
        void setData(void *data, unsigned numBytes, GLenum usage = GL_STATIC_DRAW);

        GLBuffer &obj;
    };

    GLBuffer(GLenum target);
    ~GLBuffer();

    const GLuint id;
    const GLenum target;
};