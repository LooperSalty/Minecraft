#version 330 core

in vec2  TexCoord;
in float Light;       // baked per-face brightness (0.5–1.0)
in float AO;          // ambient occlusion (0.2–1.0)
in vec3  FragNormal;
in float FragDist;
in vec3  FragPos;
in float IsWater;

out vec4 FragColor;

uniform sampler2D uTextureAtlas;
uniform vec3  uFogColor;
uniform float uFogDensity;
uniform vec3  uSunDirection;
uniform float uTime;
uniform float uSunlight;     // 0..1  (0.2 = night, 1.0 = noon)
uniform vec3  uSkyColor;     // current sky colour for ambient fill

void main() {
    vec2 tc = TexCoord;

    // ---- Water UV animation ----
    if (IsWater > 0.9) {
        float w1 = sin(uTime * 1.2 + FragPos.x * 0.8 + FragPos.z * 0.6) * 0.004;
        float w2 = cos(uTime * 0.9 + FragPos.z * 1.1 - FragPos.x * 0.4) * 0.003;
        float w3 = sin(uTime * 0.7 + FragPos.x * 1.5 - FragPos.z * 0.9) * 0.002;
        tc += vec2(w1 + w3, w2);
    }

    vec4 texColor = texture(uTextureAtlas, tc);
    if (texColor.a < 0.1) discard;

    // ---- Lighting ----
    vec3  N      = normalize(FragNormal);
    vec3  sunDir = normalize(uSunDirection);

    // Wrap-diffuse for softer shadows (Minecraft-like feel)
    float NdotL      = dot(N, sunDir);
    float wrapDiff   = max(NdotL * 0.5 + 0.5, 0.0);

    // Warm sun colour during day, cooler at night
    vec3 sunCol = mix(vec3(0.90, 0.75, 0.55), vec3(1.0, 0.97, 0.92), uSunlight);

    // Sky-aware ambient: upper faces get more sky light
    float skyWeight = max(N.y * 0.5 + 0.5, 0.0);
    vec3  ambient   = mix(vec3(0.06, 0.06, 0.10), uSkyColor * 0.28, skyWeight);
    ambient *= mix(0.30, 1.0, uSunlight);   // darken ambient at night

    // Final light contribution
    vec3 lightCol = ambient + sunCol * wrapDiff * uSunlight * 0.82;

    // Soften AO slightly to avoid harsh darkness
    float ao = mix(AO, 1.0, 0.12);

    vec3 litColor = texColor.rgb * Light * lightCol * ao;

    // ---- Water ----
    float alpha = texColor.a;
    if (IsWater > 0.9) {
        litColor *= vec3(0.28, 0.52, 0.82);

        // Caustics pattern
        float c1 = sin(uTime * 1.5 + FragPos.x * 2.0 + FragPos.z * 3.0) * 0.5 + 0.5;
        float c2 = sin(uTime * 1.1 + FragPos.x * 3.0 - FragPos.z * 1.5) * 0.5 + 0.5;
        litColor += vec3(0.015, 0.03, 0.05) * c1 * c2;

        // Specular shimmer
        vec3 perturbN = N + vec3(
            sin(uTime * 2.0 + FragPos.x * 4.0) * 0.10,
            0.0,
            cos(uTime * 1.5 + FragPos.z * 3.0) * 0.10);
        float spec = pow(max(dot(reflect(-sunDir, normalize(perturbN)),
                                 vec3(0.0, 1.0, 0.0)), 0.0), 24.0);
        litColor += spec * 0.35 * sunCol * uSunlight;

        alpha = 0.52;

    } else if (IsWater > 0.3) {
        // Glass / ice
        alpha = max(texColor.a * 1.5, 0.15);
        float gloss = pow(max(dot(reflect(-sunDir, N), vec3(0.0, 1.0, 0.0)), 0.0), 14.0) * 0.18;
        litColor += vec3(gloss) * uSunlight;
    }

    // ---- Fog ----
    float fogExp = FragDist * uFogDensity;
    float fog    = 1.0 - exp(-(fogExp * fogExp));

    // Height-fog: denser near sea level
    float hFog = exp(-max(FragPos.y - 58.0, 0.0) * 0.018) * 0.25;
    fog = clamp(fog + hFog * fog, 0.0, 1.0);

    // Sunset-tinted fog
    float sunH       = clamp(uSunDirection.y, 0.0, 1.0);
    float sunsetMix  = 1.0 - smoothstep(0.0, 0.30, sunH);
    vec3  sunsetFog  = vec3(0.92, 0.45, 0.18);
    vec3  finalFog   = mix(uFogColor, sunsetFog, sunsetMix * 0.50);

    vec3 finalColor = mix(litColor, finalFog, fog);

    FragColor = vec4(finalColor, alpha);
}
