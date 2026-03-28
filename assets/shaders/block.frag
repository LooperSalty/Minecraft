#version 330 core

in vec2 TexCoord;
in float Light;
in float FogFactor;

out vec4 FragColor;

uniform sampler2D uTextureAtlas;
uniform vec3 uFogColor;

void main() {
    vec4 texColor = texture(uTextureAtlas, TexCoord);
    if (texColor.a < 0.1) discard;

    vec3 litColor = texColor.rgb * Light;
    vec3 finalColor = mix(uFogColor, litColor, FogFactor);

    FragColor = vec4(finalColor, texColor.a);
}
