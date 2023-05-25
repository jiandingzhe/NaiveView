#version 100
#extension GL_OES_EGL_image_external : enable

uniform samplerExternalOES frame_yuv;

varying highp vec2 frag_tex_coord;

void main()
{
    gl_FragColor = texture2D(frame_yuv, frag_tex_coord);;
}