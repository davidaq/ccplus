#version 120

varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    ca.rgb *= ca.a;
    cb.rgb *= cb.a;
    vec3 multiply = (1.0 - ca.a) * cb.rgb + (1.0 - cb.a) * ca.rgb + ca.rgb * cb.rgb;
    vec3 screen =  ca.rgb + cb.rgb - ca.rgb * cb.rgb;
    float v = float(0.5 < ca.a);
    gl_FragColor.rgb = v * screen + (1.0 - v) * multiply;
    gl_FragColor.a = ca.a + cb.a - (ca.a * cb.a);
    gl_FragColor.rgb /= gl_FragColor.a;
}
