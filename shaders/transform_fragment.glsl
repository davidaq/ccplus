#version 120
attribute vec4 vertex_position;
uniform mat4 M; // This is model transformation 
uniform sampler2D tex;

void main()  {
    vec4 color = texture2D(tex, gl_TexCoord[0].st);
    gl_Position = M * vertex_position / vertex_position.w; 
}
