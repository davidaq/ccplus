attribute vec4 vertex_position;

varying vec2 xy;

uniform mat3 trans;
uniform vec4 src_dst_size; //src.width, src.height, dst.width, dst.height

void main() {
    xy = vertex_position.xy * 0.5 + 0.5;
    
    vec3 tmp = vec3(xy, 1.0);
    tmp.xy *= src_dst_size.xy;
    tmp = trans * tmp;
    tmp /= tmp.z;
    tmp.xy /= src_dst_size.zw;
    gl_Position.xy = tmp.xy * 2.0 - 1.0;
    gl_Position.zw = vertex_position.zw;
}
