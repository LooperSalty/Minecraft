#version 330 core

in vec2 TexCoord;
in float Light;
in float AO;
in vec3 FragNormal;
in float FragDist;
in vec3 FragPos;
in float IsWater;

out vec4 FragColor;

uniform sampler2D uTextureAtlas;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform vec3 uSunDirection;
uniform float uTime;

void main() {
    vec2 tc = TexCoord;

    // Animated UV offset for water
    if (IsWater > 0.9) {
        float wave1 = sin(uTime * 1.2 + FragPos.x * 0.8 + FragPos.z * 0.6) * 0.003;
        float wave2 = cos(uTime * 0.9 + FragPos.z * 1.1 - FragPos.x * 0.4) * 0.002;
        tc += vec2(wave1, wave2);
    }

    vec4 texColor = texture(uTextureAtlas, tc);

    // Discard fully transparent pixels (for vegetation, etc.)
    if (texColor.a < 0.1) discard;

    // --- Directional lighting ---
    vec3 normal = normalize(FragNormal);
    float diffuse = max(dot(normal, normalize(uSunDirection)), 0.0);
    float sunLight = 0.2 + 0.8 * diffuse;

    // Combine face brightness (baked), sun lighting, and AO
    vec3 litColor = texColor.rgb * Light * sunLight * AO;

    // --- Water tint and transparency ---
    float alpha = texColor.a;
    if (IsWater > 0.9) {
        // Apply blue tint to water
        litColor = litColor * vec3(0.4, 0.6, 1.0);
        // Animated surface shimmer
        float shimmer = sin(uTime * 2.0 + FragPos.x * 3.0 + FragPos.z * 2.5) * 0.05 + 0.05;
        litColor += vec3(shimmer * 0.3, shimmer * 0.5, shimmer * 0.8);
        alpha = 0.6;
    } else if (IsWater > 0.3) {
        // Glass and other semi-transparent blocks: keep texture alpha
        alpha = max(texColor.a * 1.5, 0.15);
        // Glass gets a subtle reflective quality
        float gloss = pow(max(dot(reflect(-normalize(uSunDirection), normal),
                                  normalize(vec3(0.0, 1.0, 0.0))), 0.0), 8.0) * 0.15;
        litColor += vec3(gloss);
    }

    // --- Exponential squared fog ---
    float fogExponent = FragDist * uFogDensity;
    float fogFactor = 1.0 - exp(-(fogExponent * fogExponent));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // --- Sunset color blending ---
    vec3 sunsetColor = vec3(0.9, 0.45, 0.2);
    float sunHeight = clamp(uSunDirection.y, 0.0, 1.0);
    float sunsetBlend = 1.0 - smoothstep(0.0, 0.3, sunHeight);
    vec3 finalFogColor = mix(uFogColor, sunsetColor, sunsetBlend * 0.5);

    // Mix lit color with fog
    vec3 finalColor = mix(litColor, finalFogColor, fogFactor);

    FragColor = vec4(finalColor, alpha);
}
