#ifdef GL_ES
precision mediump float;
varying vec4 sf_color;
varying vec2 sf_texCoord;
uniform sampler2D sf_sampler;
uniform mat4 sf_texture;
#endif
uniform float time;
uniform vec4 goldColor;
uniform vec4 whiteColor;
uniform vec4 greenColor;
uniform vec4 redColor;
void main()
{
#ifdef GL_ES
    vec4 color = sf_color;
    vec2 texCoord = sf_texCoord;
#else
    vec4 color = gl_Color;
    vec2 texCoord = gl_TexCoord[0].xy;
#endif
    if (color == redColor)
    {
        gl_FragColor = color;
    }
    else if (color == greenColor)
    {
	    vec2 coord = (texCoord + vec2(0, time * 10.0)) * 0.01;
        float light = mod(coord.x - coord.y * 0.5, 0.8) * mod(-coord.x - coord.y * 0.5, 0.8) * mod(coord.x - coord.y * 0.3, 1.8) * mod(-coord.x - coord.y * 0.2, 1.3);
        light = pow(light, 2.0) * 0.1;
        gl_FragColor = color - vec4(0, light, 0, 0);
    }
    else if (color == goldColor)
    {
        float light = abs(mod((0.5 * cos(texCoord.x * 0.2 + texCoord.y * 0.05) * 10.0 + sin(texCoord.x * 0.1 + texCoord.y * 0.2) * 2.0 + time * 20.0) * 0.2, 10.0) - 5.0) / 20.0;
        light *= light;
        gl_FragColor = color + vec4(light);
    }
    else if (color == whiteColor)
        discard;
    else
        gl_FragColor = color;
}
