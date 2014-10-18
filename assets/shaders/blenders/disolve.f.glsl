#include <noise.glsl>
varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    float val = float(snoise2D(xy) - 0.01 < ca.a);
    gl_FragColor.rgb = val * ca.rgb + (1.0 - val) * cb.rgb;
    gl_FragColor.a = 1.0;
}
