#include "Buffer.h"

#include "Utils.h"

#include <cassert>

static GLuint gen_one_buffer()
{
    GLuint re = 0;
    glGenBuffers(1, &re);
    assert(re != 0);
    return re;
}

GLBuffer::BindScope::BindScope(GLBuffer &obj) : obj(obj)
{
    glBindBuffer(obj.target, obj.id);
}

GLBuffer::BindScope::~BindScope()
{
#ifndef NDEBUG
    GLuint curr_binding = 0;
    if (obj.target == GL_ARRAY_BUFFER)
        curr_binding = get_binding(GL_ARRAY_BUFFER_BINDING);
    else if (obj.target == GL_ELEMENT_ARRAY_BUFFER)
        curr_binding = get_binding(GL_ELEMENT_ARRAY_BUFFER_BINDING);
    assert(obj.id == curr_binding);
#endif
}

void GLBuffer::BindScope::setSize(unsigned numBytes, GLenum usage)
{
    glBufferData(obj.target, numBytes, nullptr, usage);
    CHECK_GL;
}

void GLBuffer::BindScope::setData(void *data, unsigned numBytes, GLenum usage)
{
    glBufferData(obj.target, numBytes, data, usage);
    CHECK_GL;
}

GLBuffer::GLBuffer(GLenum target) : id(gen_one_buffer()), target(target)
{
    glBindBuffer(target, id);
    CHECK_GL;
}

GLBuffer::~GLBuffer()
{
    glDeleteBuffers(1, &id);
    CHECK_GL;
}