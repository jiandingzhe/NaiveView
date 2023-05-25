#pragma once

#include <cassert>

#include <epoxy/gl.h>

#define CHECK_GL assert(!check_gl_error(__FILE__, __LINE__))

bool check_gl_error(const char *file, int line);

GLuint get_binding(GLenum what);
