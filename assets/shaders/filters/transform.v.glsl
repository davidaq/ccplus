attribute vec4 vertex_position;

varying vec2 xy;

uniform mat3 trans;
uniform float src_width;
uniform float src_height;
uniform float dst_width;
uniform float dst_height;

void main() {
    xy = vertex_position.xy * 0.5 + 0.5;
    
    vec3 tmp = vec3(xy, 1.0);
    tmp.x *= src_width;
    tmp.y *= src_height;
    tmp = trans * tmp;
    tmp /= tmp.z;
    tmp.x /= dst_width;
    tmp.y /= dst_height;
    gl_Position.xy = tmp.xy * 2.0 - 1.0;
    gl_Position.zw = vertex_position.zw;
}
