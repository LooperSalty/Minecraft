#version 330 core

in vec3 vViewDir;
out vec4 FragColor;

uniform vec3  uSunDirection;
uniform vec3  uZenithColor;
uniform vec3  uHorizonColor;
uniform float uSunlight;      // 0..1  (0.2 = night, 1.0 = noon)
uniform float uTime;

void main() {
    vec3 dir = normalize(vViewDir);
    float h  = dir.y;

    // --- Sky gradient (horizon → zenith) ---
    float t = max(h, 0.0);
    vec3 sky = mix(uHorizonColor, uZenithColor, pow(t, 0.45));

    // Below-horizon darkening (ground bleed)
    if (h < 0.0) {
        vec3 ground = uHorizonColor * 0.40;
        sky = mix(uHorizonColor, ground, clamp(-h * 4.0, 0.0, 1.0));
    }

    // --- Sun ---
    vec3  sunDir  = normalize(-uSunDirection);
    float sunDot  = dot(dir, sunDir);

    // Disk
    float disk = smoothstep(0.9993, 0.9998, sunDot);
    sky += disk * vec3(1.5, 1.25, 0.95) * uSunlight;

    // Corona / glow
    float glow = pow(max(sunDot, 0.0), 200.0);
    sky += glow * vec3(1.0, 0.82, 0.45) * 0.45 * uSunlight;

    // Atmospheric scattering near horizon toward sun
    float scatter = pow(max(sunDot, 0.0), 5.0) * (1.0 - t) * 0.30;
    sky += scatter * vec3(1.0, 0.55, 0.18) * uSunlight;

    // --- Moon ---
    vec3  moonDir = -sunDir;
    float moonDot = dot(dir, moonDir);
    float nightFade = 1.0 - clamp(uSunlight * 1.5, 0.0, 1.0);
    float moon = smoothstep(0.9996, 0.9999, moonDot);
    sky += moon * vec3(0.70, 0.75, 0.90) * nightFade;

    // Subtle moon glow
    float moonGlow = pow(max(moonDot, 0.0), 128.0) * 0.12;
    sky += moonGlow * vec3(0.5, 0.55, 0.7) * nightFade;

    // --- Stars ---
    if (nightFade > 0.05) {
        vec3 sd = floor(dir * 450.0);
        float rnd = fract(sin(dot(sd, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
        if (rnd > 0.9975) {
            float bri = (rnd - 0.9975) / 0.0025;
            float twinkle = sin(uTime * (1.2 + rnd * 4.0) + rnd * 200.0) * 0.25 + 0.75;
            // Random star color temperature
            float cTemp = fract(rnd * 127.1);
            vec3 starCol = mix(vec3(0.8, 0.85, 1.0), vec3(1.0, 0.9, 0.7), cTemp);
            sky += bri * twinkle * nightFade * starCol * 0.9;
        }
    }

    // --- Simple cloud wisps ---
    if (h > 0.01) {
        float cx = dir.x / (h + 0.001) * 0.3 + uTime * 0.003;
        float cz = dir.z / (h + 0.001) * 0.3;
        float cloud  = sin(cx * 3.0 + cz * 2.0) * cos(cz * 4.0 - cx * 1.5);
        cloud = smoothstep(0.35, 0.70, cloud * 0.5 + 0.5);
        cloud *= smoothstep(0.0, 0.15, h);                  // fade at horizon
        cloud *= (0.3 + 0.7 * uSunlight);                   // darker at night
        vec3 cloudCol = mix(vec3(0.25), vec3(1.0, 0.98, 0.95), uSunlight);
        sky = mix(sky, cloudCol, cloud * 0.35);
    }

    FragColor = vec4(sky, 1.0);
}
