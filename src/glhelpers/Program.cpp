#include "Program.h"
#include <GLES2/gl2.h>
#include <SDL_opengles2.h>

#include "Utils.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using std::clog;
using std::endl;

struct GLProgram::Guts
{
    GLint get_uniform_loc(const char *name) const;
    std::unordered_map<std::string, Attr> uniforms;
    std::vector<std::pair<std::string, Attr>> attribs;
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

void GLProgram::attachShader(GLuint shader)
{
    glAttachShader(id, shader);
    CHECK_GL;
}

inline std::pair<GLenum, int> to_scalar_type_size(GLenum type)
{
    switch (type)
    {
    case GL_FLOAT:
        return {GL_FLOAT, 1};
    case GL_FLOAT_VEC2:
        return {GL_FLOAT, 2};
    case GL_FLOAT_VEC3:
        return {GL_FLOAT, 3};
    case GL_FLOAT_VEC4:
        return {GL_FLOAT, 4};
    case GL_INT:
        return {GL_INT, 1};
    case GL_UNSIGNED_INT:
        return {GL_UNSIGNED_INT, 1};
    case GL_SHORT:
        return {GL_SHORT, 1};
    case GL_UNSIGNED_SHORT:
        return {GL_UNSIGNED_SHORT, 1};
    case GL_BYTE:
        return {GL_BYTE, 1};
    case GL_UNSIGNED_BYTE:
        return {GL_UNSIGNED_BYTE, 1};
    default:
        assert(false);
        return {type, 1};
    }
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
        auto scalar_type = to_scalar_type_size(attr.type);
        attr.type = scalar_type.first;
        attr.size *= scalar_type.second;
        std::string name(name_buf, size_t(name_len));
        if (guts->attribs.size() < attr.location + 1)
            guts->attribs.resize(attr.location + 1);
        guts->attribs[attr.location] = {name, attr};
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
    for (const auto &attr : guts->attribs)
    {
        if (attr.first == name)
            return attr.second;
    }
    return {};
}

bool GLProgram::configAttribute(GLuint index, GLsizei offset, GLsizei stride, GLboolean normalized)
{
    if (index >= guts->attribs.size())
        return false;
    const auto &attrib = guts->attribs[index].second;
    assert(attrib.location == index);
    if (attrib.location < 0 || attrib.size < 1 || attrib.type == 9)
        return false;
    glEnableVertexAttribArray(index);
    CHECK_GL;
    glVertexAttribPointer(index, attrib.size, attrib.type, normalized, stride, (const void *)offset);
    CHECK_GL;
    return true;
}

const std::string &GLProgram::getLastError()
{
    return _last_error_();
}