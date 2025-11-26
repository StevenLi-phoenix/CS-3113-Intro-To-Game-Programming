#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform float u_intensity;
uniform float u_time;

void main()
{
    vec4 base = texture(texture0, fragTexCoord) * fragColor;
    float vignette = smoothstep(1.15, 0.2, length(fragTexCoord - vec2(0.5)) * 1.35);
    float wave = 0.55 + 0.45 * sin(u_time * 12.0 + length(fragTexCoord - vec2(0.5)) * 14.0);
    float blend = clamp(u_intensity * wave * vignette, 0.0, 1.0);
    vec3 tinted = mix(base.rgb, vec3(1.0, 0.1, 0.08), blend);
    float alpha = base.a * mix(0.35, 1.0, blend);
    finalColor = vec4(tinted, alpha);
}
