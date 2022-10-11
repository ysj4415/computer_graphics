#version 330 core
layout (location = 0) in vec3 in_Position; //--- ��ġ ����: attribute position 0
//layout (location = 0) in vec4 in_Position; //--- ��ġ ����: attribute position 0
layout (location = 1) in vec3 in_Color; //--- �÷� ����: attribute position 1

out vec3 out_Color;
uniform mat4 transform;
void main(void)
{
gl_Position = transform *vec4 (in_Position, 1.0f);
//gl_Position = vec4 in_Position;
out_Color = in_Color;
}