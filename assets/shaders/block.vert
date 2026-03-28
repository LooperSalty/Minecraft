#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aLight;
layout (location = 3) in vec3 aNormal;

out vec2 TexCoord;
out float Light;
out float FogFactor;

uniform mat4 uViewProjection;
uniform vec3 uCameraPos;
uniform float uFogStart;
uniform float uFogEnd;

void main() {
    gl_Position = uViewProjection * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Light = aLight;

    float dist = length(aPos - uCameraPos);
    FogFactor = clamp((uFogEnd - dist) / (uFogEnd - uFogStart), 0.0, 1.0);
}
