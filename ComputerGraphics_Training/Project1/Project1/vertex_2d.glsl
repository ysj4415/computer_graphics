#version 330 core
layout (location = 0) in vec3 in_Position; //--- ��ġ ����: attribute position 0
//layout (location = 0) in vec4 in_Position; //--- ��ġ ����: attribute position 0
void main(void)
{
gl_Position = vec4 (in_Position, 1.0f);
//gl_Position = vec4 in_Position;

}