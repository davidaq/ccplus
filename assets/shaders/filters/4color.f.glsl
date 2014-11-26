varying vec2 xy;

//uniform sampler2D tex;

uniform float params_r[6]; 
uniform float params_g[6]; 
uniform float params_b[6]; 
uniform float opacity; 

void main() {
    gl_FragColor.r = 
        params_r[0] * xy.x * xy.x + 
        params_r[1] * xy.x * xy.y +
        params_r[2] * xy.y * xy.y + 
        params_r[3] * xy.x +
        params_r[4] * xy.y +
        params_r[5];
    gl_FragColor.g = 
        params_g[0] * xy.x * xy.x + 
        params_g[1] * xy.x * xy.y +
        params_g[2] * xy.y * xy.y + 
        params_g[3] * xy.x +
        params_g[4] * xy.y +
        params_g[5];
    gl_FragColor.b = 
        params_b[0] * xy.x * xy.x + 
        params_b[1] * xy.x * xy.y +
        params_b[2] * xy.y * xy.y + 
        params_b[3] * xy.x +
        params_b[4] * xy.y +
        params_b[5];
    gl_FragColor.a = opacity;
    gl_FragColor.rgb = min(gl_FragColor.rgb, vec3(1.0));
    gl_FragColor.rgb = max(gl_FragColor.rgb, vec3(0.0)) * opacity;
}
