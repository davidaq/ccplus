varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    gl_FragColor.rgb = min((1.0 - ca.a) * cb.rgb + ca.rgb, (1.0 - cb.a) * ca.rgb + cb.rgb);
    gl_FragColor.a = ca.a + cb.a - (ca.a * cb.a);
}
