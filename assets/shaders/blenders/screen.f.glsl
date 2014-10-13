#version 120

varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    ca.rgb *= ca.a;
    cb.rgb *= cb.a;
    gl_FragColor.rgb = ca.rgb + cb.rgb - ca.rgb * cb.rgb;
    gl_FragColor.a = ca.a + cb.a - (ca.a * cb.a);
    gl_FragColor.rgb /= gl_FragColor.a;
}
