#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aLight;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in float aAO;
layout (location = 5) in float aIsWater;

out vec2 TexCoord;
out float Light;
out float AO;
out vec3 FragNormal;
out float FragDist;
out vec3 FragPos;
out float IsWater;

uniform mat4 uViewProjection;
uniform vec3 uCameraPos;

void main() {
    gl_Position = uViewProjection * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Light = aLight;
    AO = aAO;
    FragNormal = aNormal;
    FragDist = length(aPos - uCameraPos);
    FragPos = aPos;
    IsWater = aIsWater;
}
