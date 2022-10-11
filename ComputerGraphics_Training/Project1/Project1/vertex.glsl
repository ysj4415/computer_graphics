#version 330 core
layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
//layout (location = 0) in vec4 in_Position; //--- 위치 변수: attribute position 0
uniform mat4 transform;
void main(void)
{
gl_Position = transform *vec4 (in_Position, 1.0f);
//gl_Position = vec4 in_Position;

}