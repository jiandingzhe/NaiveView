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
        void setUniform(GLint loc, GLfloat n1);
        void setUniform(GLint loc, GLint n1);
        void setUniform(GLint loc, GLfloat n1, GLfloat n2);
        void setUniform(GLint loc, GLfloat n1, GLfloat n2, GLfloat n3);
        void setUniform(GLint loc, GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4);
        void setUniform(GLint loc, GLint n1, GLint n2);
        void setUniform(GLint loc, GLint n1, GLint n2, GLint n3);
        void setUniform(GLint loc, GLint n1, GLint n2, GLint n3, GLint n4);
        void setUniform(GLint loc, const GLfloat *values, GLsizei numValues);
        void setUniformMat2(GLint loc, const GLfloat *v, GLint numMatrix, GLboolean trns);
        void setUniformMat3(GLint loc, const GLfloat *v, GLint numMatrix, GLboolean trns);
        void setUniformMat4(GLint loc, const GLfloat *v, GLint numMatrix, GLboolean trns);
        GLProgram &obj;
    };
    struct Attr
    {
        GLint location = -1;
        GLint size = 0;
        GLenum type = 0;
    };

    GLProgram();
    GLProgram(const GLProgram &) = delete;
    ~GLProgram();

    void attachShader(GLuint shader);
    bool link();

    Attr getAttribute(const char *name) const;
    bool configAttribute(GLuint index, GLsizei offset, GLsizei stride, GLboolean normalized = GL_FALSE);

    Attr getUniform(const char *name) const;

    static const std::string &getLastError();

    const GLuint id;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};