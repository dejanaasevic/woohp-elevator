#version 330 core

in vec4 chCol;
in vec2 chTex;
out vec4 outCol;

uniform sampler2D uTex;
uniform bool uUseTex;

void main()
{
    if (uUseTex) {
        outCol = texture(uTex, chTex);
    } else {
        outCol = chCol;
    }
}