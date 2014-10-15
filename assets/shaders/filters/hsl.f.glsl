#version 120

varying vec2 xy;

uniform sampler2D tex;
uniform float hue; // 0 ~ 1
uniform float sat; // 0 ~ 2.0
uniform float lit; // 0 ~ 2.0

float min3(float r, float g, float b) {
    return min(r, min(g, b));
}

float max3(float r, float g, float b) {
    return max(r, max(g, b));
}

vec4 convertRGBtoHSL( vec4 col )
{
    float red   = col.r;
    float green = col.g;
    float blue  = col.b;

    float minc  = min3( col.r, col.g, col.b );
    float maxc  = max3( col.r, col.g, col.b );
    float delta = maxc - minc;

    float lum = (minc + maxc) * 0.5;
    float sat = 0.0;
    float hue = 0.0;

    if (lum > 0.0 && lum < 1.0) {
        float mul = (lum < 0.5)  ?  (lum)  :  (1.0-lum);
        sat = delta / (mul * 2.0);
    }

    vec3 masks = vec3(
        (maxc == red   && maxc != green) ? 1.0 : 0.0,
        (maxc == green && maxc != blue)  ? 1.0 : 0.0,
        (maxc == blue  && maxc != red)   ? 1.0 : 0.0
    );

    vec3 adds = vec3(
              ((green - blue ) / delta),
        2.0 + ((blue  - red  ) / delta),
        4.0 + ((red   - green) / delta)
    );

    float deltaGtz = (delta > 0.0) ? 1.0 : 0.0;

    hue += dot( adds, masks );
    hue *= deltaGtz;
    hue /= 6.0;

    if (hue < 0.0)
        hue += 1.0;

    return vec4( hue, sat, lum, col.a );
}

// HSL [0:1] to RGB [0:1]
vec4 convertHSLtoRGB( vec4 col )
{
    const float onethird = 1.0 / 3.0;
    const float twothird = 2.0 / 3.0;
    const float rcpsixth = 6.0;

    float hue = col.x;
    float sat = col.y;
    float lum = col.z;

    vec3 xt = vec3(
        rcpsixth * (hue - twothird),
        0.0,
        rcpsixth * (1.0 - hue)
    );

    if (hue < twothird) {
        xt.r = 0.0;
        xt.g = rcpsixth * (twothird - hue);
        xt.b = rcpsixth * (hue      - onethird);
    } 

    if (hue < onethird) {
        xt.r = rcpsixth * (onethird - hue);
        xt.g = rcpsixth * hue;
        xt.b = 0.0;
    }

    xt = min( xt, 1.0 );

    float sat2   =  2.0 * sat;
    float satinv =  1.0 - sat;
    float luminv =  1.0 - lum;
    float lum2m1 = (2.0 * lum) - 1.0;
    vec3  ct     = (sat2 * xt) + satinv;

    vec3 rgb;
    if (lum >= 0.5)
         rgb = (luminv * ct) + lum2m1;
    else rgb =  lum    * ct;

    return vec4( rgb, col.a );
}

void main() {
    vec4 col = texture2D(tex, xy);
    vec4 hsl = convertRGBtoHSL(col);
    // Hue
    hsl.r += hue;
    if (hsl.r > 1)
        hsl.r -= 1.0;
    if (hsl.r < 0)
        hsl.r += 1.0;
    //// Sat
    hsl.g += (sat - 1.0) * (sat > 1.0 ? (1.0 - hsl.g) : hsl.g);
    hsl.g = max(0, min(1, hsl.g));
    //// Lit
    hsl.b += (lit - 1.0) * (lit > 1.0 ? (1.0 - hsl.b) : hsl.b);
    hsl.b = max(0, min(1, hsl.b));

    gl_FragColor = convertHSLtoRGB(hsl);
}
