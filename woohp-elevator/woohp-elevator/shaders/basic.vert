#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inCol;
layout(location = 2) in vec2 inTex;
out vec4 chCol;
out vec2 chTex;

uniform vec2 uPos;
uniform bool uFlip;  

void main()
{
    gl_Position = vec4(inPos.x + uPos.x, inPos.y + uPos.y, 0.0, 1.0);
    chCol = vec4(inCol, 1.0);
    if(uFlip){
        chTex = vec2(1.0 - inTex.x, inTex.y); 
    }
    else{
        chTex = inTex;
    }
}