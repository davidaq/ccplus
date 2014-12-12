varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    gl_FragColor = cb * (1.0 - ca.a);
}
