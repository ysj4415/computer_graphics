#version 330 core
layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
//layout (location = 0) in vec4 in_Position; //--- 위치 변수: attribute position 0

void main(void)
{
gl_Position = vec4 (in_Position.x, in_Position.y, in_Position.z, 1.0);
//gl_Position = vec4 in_Position;

}