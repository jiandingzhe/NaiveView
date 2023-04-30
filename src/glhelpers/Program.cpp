#include "Program.h"
#include <GLES2/gl2.h>

#include "Utils.h"

#include <iostream>
#include <string>
#include <unordered_map>

using std::clog;
using std::endl;

struct GLProgram::Guts
{
    GLint get_uniform_loc(const char *name) const;
    std::unordered_map<std::string, Attr> uniforms;
    std::unordered_map<std::string, Attr> attribs;
};

GLint GLProgram::Guts::get_uniform_loc(const char *name) const
{
    auto i_uni = uniforms.find(name);
    if (i_uni == uniforms.end())
        return -1;
    else
        return i_uni->second.location;
}

GLProgram::BindScope::BindScope(GLProgram &obj) : obj(obj)
{
    glUseProgram(obj.id);
}

GLProgram::BindScope::~BindScope()
{
    auto curr_prog = get_binding(GL_CURRENT_PROGRAM);
    assert(obj.id == curr_prog);
}

void GLProgram::BindScope::setUniform(const char *name, GLfloat n1)
{
    glUniform1f(obj.guts->get_uniform_loc(name), n1);
}
void GLProgram::BindScope::setUniform(const char *name, GLint n1)
{
    glUniform1i(obj.guts->get_uniform_loc(name), n1);
}
void GLProgram::BindScope::setUniform(const char *name, GLfloat n1, GLfloat n2)
{
    glUniform2f(obj.guts->get_uniform_loc(name), n1, n2);
}
void GLProgram::BindScope::setUniform(const char *name, GLfloat n1, GLfloat n2, GLfloat n3)
{
    glUniform3f(obj.guts->get_uniform_loc(name), n1, n2, n3);
}
void GLProgram::BindScope::setUniform(const char *name, GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4)
{
    glUniform4f(obj.guts->get_uniform_loc(name), n1, n2, n3, n4);
}
void GLProgram::BindScope::setUniform(const char *name, GLint n1, GLint n2)
{
    glUniform2i(obj.guts->get_uniform_loc(name), n1, n2);
}
void GLProgram::BindScope::setUniform(const char *name, GLint n1, GLint n2, GLint n3)
{
    glUniform3i(obj.guts->get_uniform_loc(name), n1, n2, n3);
}
void GLProgram::BindScope::setUniform(const char *name, GLint n1, GLint n2, GLint n3, GLint n4)
{
    glUniform4i(obj.guts->get_uniform_loc(name), n1, n2, n3, n4);
}
void GLProgram::BindScope::setUniform(const char *name, const GLfloat *values, GLsizei numValues)
{
    glUniform1fv(obj.guts->get_uniform_loc(name), numValues, values);
}
void GLProgram::BindScope::setUniformMat2(const char *name, const GLfloat *v, GLint num, GLboolean trns)
{
    glUniformMatrix2fv(obj.guts->get_uniform_loc(name), num, trns, v);
}
void GLProgram::BindScope::setUniformMat3(const char *name, const GLfloat *v, GLint num, GLboolean trns)
{
    glUniformMatrix3fv(obj.guts->get_uniform_loc(name), num, trns, v);
}
void GLProgram::BindScope::setUniformMat4(const char *name, const GLfloat *v, GLint num, GLboolean trns)
{
    glUniformMatrix4fv(obj.guts->get_uniform_loc(name), num, trns, v);
}

GLProgram::GLProgram() : id(glCreateProgram()), guts(new Guts)
{
    CHECK_GL;
}

GLProgram::~GLProgram()
{
    glDeleteProgram(id);
    CHECK_GL;
}

static std::string &_last_error_()
{
    static std::string error;
    return error;
}

bool GLProgram::link()
{
    glLinkProgram(id);

    // get program link status
    GLint status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLchar infoLog[4096] = {};
        GLsizei infoLogLength = 0;
        glGetProgramInfoLog(id, sizeof(infoLog), &infoLogLength, infoLog);
        _last_error_() = std::string(infoLog, (size_t)infoLogLength);
        std::clog << _last_error_() << std::endl;
        return false;
    }

    // query program attributes
    guts->attribs.clear();
    GLint n_attr = 0;
    glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &n_attr);
    clog << "GLSL program #" << id << " has " << n_attr << " attributes:" << endl;
    GLchar name_buf[255];

    for (int i_attr = 0; i_attr < n_attr; i_attr++)
    {
        GLsizei name_len;
        Attr attr;
        glGetActiveAttrib(id, i_attr, sizeof(name_buf), &name_len, &attr.size, &attr.type, name_buf);
        attr.location = glGetAttribLocation(id, name_buf);
        std::string name(name_buf, size_t(name_len));
        guts->attribs.insert({name, attr});
    }
    CHECK_GL;

    // query program uniforms
    guts->uniforms.clear();
    GLuint n_uni = 0;
    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, (GLint *)&n_uni);
    for (GLuint i_uni = 0; i_uni < n_uni; i_uni += 1)
    {
        GLsizei name_len;
        Attr attr;
        glGetActiveUniform(id, i_uni, sizeof(name_buf), &name_len, &attr.size, &attr.type, name_buf);
        attr.location = glGetUniformLocation(id, name_buf);
        std::string name(name_buf, size_t(name_len));
        guts->uniforms.insert({name, attr});
    }
    CHECK_GL;

    return true;
}

GLProgram::Attr GLProgram::getAttribute(const char *name) const
{
    auto i_attr = guts->attribs.find(std::string(name));
    if (i_attr == guts->attribs.end())
        return {};
    else
        return i_attr->second;
}