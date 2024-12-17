#ifdef GL_ES
attribute vec2 position;
attribute vec4 color;
attribute vec2 texCoord;
varying vec4 sf_color;
varying vec2 sf_texCoord;
uniform mat4 sf_modelview;
uniform mat4 sf_projection;
void main()
{
    sf_color = color;
    sf_texCoord = texCoord;
    vec4 pos = sf_projection * sf_modelview * vec4(position, 0.0, 1.0);
    gl_Position = pos;
}
#else
void main()
{
    vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position = pos;
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    gl_FrontColor = gl_Color;
}
#endif