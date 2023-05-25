#include <SDL.h>

#include <cstring>
#include <iostream>
#include <thread>

#include "glhelpers/Buffer.h"
#include "glhelpers/Program.h"
#include "glhelpers/Shader.h"
#include "glhelpers/Utils.h"

using namespace std::chrono_literals;
using std::clog;
using std::endl;

struct TestVert
{
    float x;
    float y;
    float s;
    float t;
};

TestVert rect[4] = {
    {-0.5f, -0.5f, 0.0f, 0.0f}, {0.5f, -0.5f, 1.0f, 0.0f}, {-0.5f, 0.5f, 0.0f, 1.0f}, {0.5f, 0.5f, 1.0f, 1.0f}};

const char *src_vert = "#version 100\n"
                       "attribute highp vec2 position;\n"
                       "attribute highp vec2 tex_coord;\n"
                       "varying highp vec2 frag_tex_coord;\n"
                       "void main()\n"
                       "{\n"
                       "    gl_Position = vec4(position, 0, 1);\n"
                       "    frag_tex_coord = tex_coord;\n"
                       "}\n";

const char *src_frag = "#version 100\n"
                       "varying highp vec2 frag_tex_coord;\n"
                       "void main()\n"
                       "{\n"
                       "    gl_FragColor = vec4(frag_tex_coord,0,1);\n"
                       "}\n";

struct GLStuffs
{
    GLBuffer buf{GL_ARRAY_BUFFER};
    GLShader vert_shader{GL_VERTEX_SHADER};
    GLShader frag_shader{GL_FRAGMENT_SHADER};
    GLProgram prog;
};

int main()
{
    auto init_re = SDL_VideoInit("KMSDRM");
    if (init_re != 0)
        exit(EXIT_FAILURE);

    SDL_Window *window = SDL_CreateWindow("SDL2", 0, 0, 1080, 1920, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (nullptr == window)
    {
        std::cerr << "SDL_CreateWindow(): " << SDL_GetError() << '\n';
        exit(EXIT_FAILURE);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    auto gl = SDL_GL_CreateContext(window);
    clog << "create OpenGL context " << gl << endl;
    auto make_current_re = SDL_GL_MakeCurrent(window, gl);
    clog << "make current ctx returned " << make_current_re << endl;
    clog << "OpenGL version: " << (const char *)glGetString(GL_VERSION) << ", GLSL "
         << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    // setup GL objects
    std::unique_ptr<GLStuffs> gl_stuffs(new GLStuffs);
    if (!gl_stuffs->vert_shader.compile(src_vert))
    {
        clog << "failed to compile vertex shader:" << endl << GLShader::getLastError() << endl;
        exit(EXIT_FAILURE);
    }

    if (!gl_stuffs->frag_shader.compile(src_frag))
    {
        clog << "failed to compile fragment shader:" << endl << GLShader::getLastError() << endl;
        exit(EXIT_FAILURE);
    }

    gl_stuffs->prog.attachShader(gl_stuffs->vert_shader.id);
    gl_stuffs->prog.attachShader(gl_stuffs->frag_shader.id);
    if (!gl_stuffs->prog.link())
    {
        clog << "failed to link program:" << endl << GLProgram::getLastError() << endl;
        exit(EXIT_FAILURE);
    }
    auto attr_pos = gl_stuffs->prog.getAttribute("position");
    auto attr_tex_coord = gl_stuffs->prog.getAttribute("tex_coord");

    {
        GLBuffer::BindScope scope(gl_stuffs->buf);
        scope.setData(rect, sizeof(rect));
    }

    // run render loop
    glEnable(GL_SCISSOR_TEST);
    CHECK_GL;

    size_t frame_count = 0;
    SDL_Event e;
    bool should_break = false;
    while (!should_break)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                should_break = true;
            else if (e.type == SDL_KEYDOWN)
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    should_break = true;
        }
        frame_count += 1;
        glScissor(0, 0, 1080, 1920);
        glClearColor(0.3, 0.3, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        {
            GLProgram::BindScope prog_scope(gl_stuffs->prog);
            GLBuffer::BindScope buffer_scope(gl_stuffs->buf);

            gl_stuffs->prog.configAttribute(attr_pos.location, 0, sizeof(TestVert));
            gl_stuffs->prog.configAttribute(attr_tex_coord.location, 8, sizeof(TestVert));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            CHECK_GL;
        }

        glScissor(0, 0, 20, 20);
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glScissor(1060, 1900, 20, 20);
        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
        CHECK_GL;
        clog << "frame " << frame_count << endl;
        std::this_thread::sleep_for(10ms);
    }

    gl_stuffs.reset();
    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
