#version 100

attribute highp vec2 position;
attribute highp vec2 tex_coord;
varying highp vec2 frag_tex_coord;

void main()
{
    gl_Position = vec4(position, 0, 1);
    frag_tex_coord = tex_coord;
}