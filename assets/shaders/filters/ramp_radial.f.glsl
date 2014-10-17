#version 120

varying vec2 xy;

uniform sampler2D tex;

uniform float dis;
uniform float alpha;
uniform vec2 start;
uniform vec2 end;
uniform vec3 s_rgb;
uniform vec3 e_rgb;

float intensity(vec2 p) {
    vec2 d = p - start;
    return sqrt(dot(d, d)) / dis;
}

void main() {
    vec3 diff_rgb = e_rgb - s_rgb;

    gl_FragColor.rgb = diff_rgb * intensity(xy) + s_rgb;
    gl_FragColor.rgb = max(vec3(0.0), gl_FragColor.rgb);
    gl_FragColor.rgb = min(vec3(1.0), gl_FragColor.rgb);
    gl_FragColor.a = alpha;
}
