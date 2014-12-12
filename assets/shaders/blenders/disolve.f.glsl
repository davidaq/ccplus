#include <noise.glsl>
varying vec2 xy;

uniform sampler2D tex_up;
uniform sampler2D tex_down;

void main() {
    vec4 ca = texture2D(tex_up, xy);
    vec4 cb = texture2D(tex_down, xy);
    float val = step(snoise2D(xy), ca.a);
    ca.a = 1.0;
    gl_FragColor = mix(cb, ca, val);
}
