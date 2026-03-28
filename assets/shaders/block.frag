#version 330 core

in vec2 TexCoord;
in float Light;
in float AO;
in vec3 FragNormal;
in float FragDist;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D uTextureAtlas;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform vec3 uSunDirection;
uniform float uTime;

void main() {
    vec4 texColor = texture(uTextureAtlas, TexCoord);
    if (texColor.a < 0.1) discard;

    // --- Directional lighting ---
    // Diffuse from sun direction with a minimum ambient of 0.2
    vec3 normal = normalize(FragNormal);
    float diffuse = max(dot(normal, normalize(uSunDirection)), 0.0);
    // Remap to [0.2, 1.0] range so shadowed faces still get some light
    float sunLight = 0.2 + 0.8 * diffuse;

    // Combine face brightness (baked), sun lighting, and AO
    vec3 litColor = texColor.rgb * Light * sunLight * AO;

    // --- Exponential squared fog ---
    float fogExponent = FragDist * uFogDensity;
    float fogFactor = 1.0 - exp(-(fogExponent * fogExponent));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // --- Sunset color blending ---
    // When sun is low (sunDirection.y close to 0), blend fog toward warm orange
    vec3 sunsetColor = vec3(0.9, 0.45, 0.2);
    float sunHeight = clamp(uSunDirection.y, 0.0, 1.0);
    // sunsetBlend is strongest when sun is near horizon (sunHeight ~ 0.0-0.15)
    float sunsetBlend = 1.0 - smoothstep(0.0, 0.3, sunHeight);
    vec3 finalFogColor = mix(uFogColor, sunsetColor, sunsetBlend * 0.5);

    // Mix lit color with fog
    vec3 finalColor = mix(litColor, finalFogColor, fogFactor);

    FragColor = vec4(finalColor, texColor.a);
}
