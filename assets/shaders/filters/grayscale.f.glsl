varying vec2 xy;

uniform float weights[6];
uniform vec2 hue_sat;
uniform sampler2D tex;

const float delta = 0.16662;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    vec4 col = texture2D(tex, xy);
    col.rgb = rgb2hsv(col.rgb);

    float idx = floor(col.r / delta);

    float upper = (idx + 1.0) * delta;
    float lower = idx * delta;

    float tmp = (upper - col.r) / delta * weights[int(mod(idx, 6.0))];
    tmp += (col.r - lower) / delta * weights[int(mod(idx + 1.0, 6.0))];
    col.r = hue_sat[0]; // hue
    col.g = hue_sat[1]; // sat
    col.b = clamp(tmp * col.b, 0.0, 1.0); // val

    gl_FragColor = vec4(hsv2rgb(col.rgb), col.a);
}
