#version 120

varying vec2 xy;

uniform sampler2D tex;

uniform vec2 pixelOffset;
uniform int ksize;

vec3 GaussianBlur(sampler2D tex0, vec2 centreUV, vec2 pixelOffset, int size)
{                                                                              
    vec3 colOut = vec3( 0, 0, 0 );
    // Kernel width 35 x 35
    //
    //const int stepCount = 9;
    // FIXME: might be slow operation
    //int stepCount = min(size, 9);
    int stepCount = size;
    //
    const float gWeights[9] = float[9](
        0.10855,
        0.13135,
        0.10406,
        0.07216,
        0.04380,
        0.02328,
        0.01083,
        0.00441,
        0.00157
    );
    const float gOffsets[9] = float[9](
        0.66293,
        2.47904,
        4.46232,
        6.44568,
        8.42917,
        10.41281,
        12.39664,
        14.38070,
        16.36501
    );

    for( int i = 0; i < stepCount; i++) {
        vec2 texCoordOffset = gOffsets[i] * pixelOffset;
        vec3 col = texture2D( tex0, centreUV + texCoordOffset ).xyz + 
            texture2D( tex0, centreUV - texCoordOffset ).xyz;
        colOut += gWeights[i] * col;                                             
    }; 
    return colOut;
}                                                                                

void main() {
    gl_FragColor.xyz = GaussianBlur(tex, xy, pixelOffset, ksize);
    gl_FragColor.w = 1;
}
