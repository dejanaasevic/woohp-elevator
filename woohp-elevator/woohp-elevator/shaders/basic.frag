#version 330 core

in vec4 chCol;
in vec2 chTex;
out vec4 outCol;

uniform sampler2D uTex;
uniform bool uUseTex;
uniform float uAlpha;

void main()
{
    vec4 color = uUseTex ? texture(uTex, chTex) : chCol;
    outCol = vec4(color.rgb, color.a * uAlpha);
}