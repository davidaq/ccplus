varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;
uniform float ratio;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy) * 0.8;
    gl_FragColor = ca + (1.0 - ca.a) * cb;
}
