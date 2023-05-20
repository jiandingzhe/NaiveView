#version 100

uniform sampler2D frame_rgb;
varying highp vec2 frag_tex_coord;

void main()
{
    mediump vec4 val = texture2D(frame_rgb, frag_tex_coord);
    gl_FragColor = vec4(val.b, val.g, val.r, 1);
}