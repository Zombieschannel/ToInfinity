#version 110

uniform float time;
uniform vec4 goldColor;
uniform vec4 whiteColor;
uniform vec4 greenColor;
uniform vec4 redColor;
void main()
{
    if (gl_Color == redColor)
    {
        gl_FragColor = gl_Color;
    }
    else if (gl_Color == greenColor)
    {
	vec2 coord = (gl_TexCoord[0].xy + vec2(0, time * 10)) * 0.01;
        float light = mod(coord.x - coord.y * 0.5, 0.8) * mod(-coord.x - coord.y * 0.5, 0.8) * mod(coord.x - coord.y * 0.3, 1.8) * mod(-coord.x - coord.y * 0.2, 1.3);
        light = pow(light, 2) * 0.1;
        gl_FragColor = gl_Color - vec4(0, light, 0, 0);
    }
    else if (gl_Color == goldColor)
    {
        float light = abs(mod((0.5 * cos(gl_TexCoord[0].x * 0.2 + gl_TexCoord[0].y * 0.05) * 10 + sin(gl_TexCoord[0].x * 0.1 + gl_TexCoord[0].y * 0.2) * 2 + time * 20) * 0.2, 10.0) - 5) / 20;
        light *= light;
        gl_FragColor = gl_Color + vec4(light);
    }
    else if (gl_Color == whiteColor)
        discard;
    else
        gl_FragColor = gl_Color;
}