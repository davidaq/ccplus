varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    vec3 multiply = (1.0 - ca.a) * cb.rgb + (1.0 - cb.a) * ca.rgb + ca.rgb * cb.rgb;
    vec3 screen =  ca.rgb + cb.rgb - ca.rgb * cb.rgb;
    gl_FragColor.rgb = mix(multiply, screen, step(0.5, ca.a));
    gl_FragColor.a = ca.a + cb.a - (ca.a * cb.a);
}
