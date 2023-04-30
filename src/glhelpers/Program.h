#pragma once

#include <GLES2/gl2.h>
#include <SDL_opengles2.h>

#include <memory>

class GLProgram
{
  public:
    struct BindScope
    {
        BindScope(GLProgram &);
        ~BindScope();
        void setUniform( const char* name, GLfloat n1 );
        void setUniform( const char* name, GLint n1 );
        void setUniform( const char* name, GLfloat n1, GLfloat n2 );
        void setUniform( const char* name, GLfloat n1, GLfloat n2, GLfloat n3 );
        void setUniform( const char* name, GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4 );
        void setUniform( const char* name, GLint n1, GLint n2 );
        void setUniform( const char* name, GLint n1, GLint n2, GLint n3 );
        void setUniform( const char* name, GLint n1, GLint n2, GLint n3, GLint n4 );
        void setUniform( const char* name, const GLfloat* values, GLsizei numValues );
        void setUniformMat2( const char* name, const GLfloat* v, GLint num, GLboolean trns );
        void setUniformMat3( const char* name, const GLfloat* v, GLint num, GLboolean trns );
        void setUniformMat4( const char* name, const GLfloat* v, GLint num, GLboolean trns );
        GLProgram &obj;
    };
    struct Attr
    {
        GLint location = -1;
        GLint size = 0;
        GLenum type = 0;
    };

    GLProgram();
    ~GLProgram();

    void attachShader(GLuint shader);
    bool link();

    Attr getAttribute(const char *name) const;

    static const std::string &getLastError();

    const GLuint id;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};