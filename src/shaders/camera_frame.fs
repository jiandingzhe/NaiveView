#version 100

sampler2D frame_y;
sampler2D frame_u;
sampler2D frame_v;

varying highp vec2 frag_tex_coord;

void main()
{
    float comp_y = texture2D(frame_y, frag_tex_coord).a;
    float comp_u = texture2D(frame_u, frag_tex_coord).a;
    float comp_v = texture2D(frame_v, frag_tex_coord).a;
    float r = comp_y + 1.14 * comp_v;
    float g = comp_y - 0.395 * comp_u - 0.581 * comp_v;
    float b = comp_y + 2.032 * comp_u;
    gl_FragColor = vec4(r, g, b, 1);
}