varying vec2 xy;

uniform sampler2D tex;
uniform float hue; // 0 ~ 1
uniform float sat; // 0 ~ 2.0
uniform float lit; // 0 ~ 2.0

vec3 HUEtoRGB(float H) {
    float R = clamp(abs(H * 6.0 - 3.0) - 1.0, 0.0, 1.0);
    float G = clamp(2 - abs(H * 6.0 - 2.0), 0.0, 1.0);
    float B = clamp(2 - abs(H * 6.0 - 4.0), 0.0, 1.0);
    return vec3(R,G,B);
}

const float Epsilon = 0.00001;
vec3 RGBtoHCV(vec3 RGB) {
    vec4 P = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0/3.0) : vec4(RGB.gb, 0.0, -1.0/3.0);
    vec4 Q = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
    float C = Q.x - min(Q.w, Q.y);
    float H = abs((Q.w - Q.y) / (6.0 * C + Epsilon) + Q.z);
    return vec3(H, C, Q.x);
}

vec3 HSLtoRGB(vec3 HSL) {
    if(HSL.g < Epsilon)
        return vec3(HSL.b);
    vec3 RGB = HUEtoRGB(HSL.x);
    float C = (1.0 - abs(2.0 * HSL.z - 1.0)) * HSL.y;
    return (RGB - 0.5) * C + HSL.z;
}

vec3 RGBtoHSL(vec3 RGB) {
    vec3 HCV = RGBtoHCV(RGB);
    float L = HCV.z - HCV.y * 0.5;
    float S = HCV.y / (1.0 - abs(L * 2.0 - 1.0) + Epsilon);
    return vec3(HCV.x, S, L);
}

void main() {
    vec4 col = texture2D(tex, xy);
    vec3 hsl = RGBtoHSL(col.rgb / col.a);
    // Hue
    hsl.r += hue;
    if (hsl.r > 1.0)
        hsl.r -= 1.0;
    if (hsl.r < 0.0)
        hsl.r += 1.0;
    // Sat
    hsl.g += (sat - 1.0) * (sat > 1.0 ? (1.0 - hsl.g) : hsl.g);
    hsl.g = clamp(hsl.g, 0.0, 1.0);
    // Lit
    hsl.b += (lit - 1.0) * (lit > 1.0 ? (1.0 - hsl.b) : hsl.b);
    hsl.b = clamp(hsl.b, 0.0, 1.0);

    gl_FragColor.rgb = HSLtoRGB(hsl) * col.a;
    gl_FragColor.a = col.a;
}
