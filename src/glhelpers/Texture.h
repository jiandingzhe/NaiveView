#pragma once

#include <epoxy/gl.h>

class GLTexture2D
{
  public:
    class BindScope
    {
      public:
        BindScope(GLTexture2D &);
        ~BindScope();

        void setMono8Image(int w, int h, const void *data);
        void setRGBImage(int w, int h, const void *data);

        GLTexture2D &obj;
    };

    GLTexture2D(GLenum type, bool mag_linear = true, bool min_linear = true, bool mipmap = true);
    GLTexture2D(const GLTexture2D &) = delete;
    ~GLTexture2D();

    const GLuint id;
    const GLenum type;
};
