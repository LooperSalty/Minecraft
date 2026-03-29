#version 330 core

layout (location = 0) in vec2 aPos;

out vec3 vViewDir;

uniform mat4 uInvVP;
uniform vec3 uCameraPos;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    // Reconstruct world-space view direction from clip-space corner
    vec4 farPt = uInvVP * vec4(aPos, 1.0, 1.0);
    farPt /= farPt.w;
    vViewDir = farPt.xyz - uCameraPos;
}
