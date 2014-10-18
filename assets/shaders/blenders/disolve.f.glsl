varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

float rand(vec2 x) {
    mediump float n = floor(x.x * 800.0 + x.y * 640000.0);
    n = mod(n * (n + 1530.0), 19747.0);
    n = mod(n * (n + 1530.0), 19747.0);
    return n / 19747.0;
}

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    float val = float(rand(xy) - 0.01 < ca.a);
    gl_FragColor.rgb = val * ca.rgb + (1.0 - val) * cb.rgb;
    gl_FragColor.a = 1.0;
}
