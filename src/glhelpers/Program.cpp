#include "Program.h"

#include "Utils.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using std::clog;
using std::endl;

typedef std::vector<std::pair<std::string, GLProgram::Attr>> AttrArray;

void ensure_size(AttrArray &arr, int expect)
{
    if (arr.size() < size_t(expect))
        arr.resize(size_t(expect));
}

struct GLProgram::Guts
{
    AttrArray uniforms;
    AttrArray attribs;
};

GLProgram::BindScope::BindScope(GLProgram &obj) : obj(obj)
{
    glUseProgram(obj.id);
}

GLProgram::BindScope::~BindScope()
{
    auto curr_prog = get_binding(GL_CURRENT_PROGRAM);
    assert(obj.id == curr_prog);
}

void GLProgram::BindScope::setUniform(GLint loc, GLfloat n1)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == 1);
        assert(obj.guts->uniforms[loc].second.type == GL_FLOAT);
        glUniform1f(loc, n1);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, GLint n1)
{
    if (loc >= 0)
    {
        const auto &meta = obj.guts->uniforms[loc].second;
        assert(meta.size == 1);
        assert(meta.type == GL_INT || meta.type == GL_SAMPLER_2D || meta.type == GL_SAMPLER_CUBE || meta.type == GL_SAMPLER_EXTERNAL_OES);
        glUniform1i(loc, n1);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, GLfloat n1, GLfloat n2)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == 2);
        assert(obj.guts->uniforms[loc].second.type == GL_FLOAT);
        glUniform2f(loc, n1, n2);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, GLfloat n1, GLfloat n2, GLfloat n3)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == 3);
        assert(obj.guts->uniforms[loc].second.type == GL_FLOAT);
        glUniform3f(loc, n1, n2, n3);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == 4);
        assert(obj.guts->uniforms[loc].second.type == GL_FLOAT);
        glUniform4f(loc, n1, n2, n3, n4);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, GLint n1, GLint n2)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == 2);
        assert(obj.guts->uniforms[loc].second.type == GL_INT);
        glUniform2i(loc, n1, n2);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, GLint n1, GLint n2, GLint n3)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == 3);
        assert(obj.guts->uniforms[loc].second.type == GL_INT);
        glUniform3i(loc, n1, n2, n3);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, GLint n1, GLint n2, GLint n3, GLint n4)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == 4);
        assert(obj.guts->uniforms[loc].second.type == GL_INT);
        glUniform4i(loc, n1, n2, n3, n4);
    }
}
void GLProgram::BindScope::setUniform(GLint loc, const GLfloat *values, GLsizei numValues)
{
    if (loc >= 0)
    {
        assert(obj.guts->uniforms[loc].second.size == numValues);
        assert(obj.guts->uniforms[loc].second.type == GL_FLOAT);
        glUniform1fv(loc, numValues, values);
    }
}
void GLProgram::BindScope::setUniformMat2(GLint loc, const GLfloat *v, GLint num, GLboolean trns)
{
    glUniformMatrix2fv(loc, num, trns, v);
}
void GLProgram::BindScope::setUniformMat3(GLint loc, const GLfloat *v, GLint num, GLboolean trns)
{
    glUniformMatrix3fv(loc, num, trns, v);
}
void GLProgram::BindScope::setUniformMat4(GLint loc, const GLfloat *v, GLint num, GLboolean trns)
{
    glUniformMatrix4fv(loc, num, trns, v);
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
    case GL_SAMPLER_2D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_EXTERNAL_OES:
        return {type, 1};
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
        return false;
    }

    // query program attributes
    guts->attribs.clear();
    GLint n_attr = 0;
    glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &n_attr);
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
        ensure_size(guts->attribs, attr.location + 1);
        guts->attribs[size_t(attr.location)] = {name, attr};
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
        auto scalar_type = to_scalar_type_size(attr.type);
        attr.type = scalar_type.first;
        attr.size *= scalar_type.second;
        std::string name(name_buf, size_t(name_len));
        ensure_size(guts->uniforms, attr.location + 1);
        guts->uniforms[size_t(attr.location)] = {name, attr};
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

GLProgram::Attr GLProgram::getUniform(const char *name) const
{
    for (const auto &uni : guts->uniforms)
    {
        if (uni.first == name)
            return uni.second;
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