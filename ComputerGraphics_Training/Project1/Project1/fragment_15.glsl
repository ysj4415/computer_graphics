#version 330 core
//--- out_Color: ���ؽ� ���̴����� �Է¹޴� ���� ��
//--- FragColor: ����� ������ ������ ������ ���۷� ���� ��.

uniform vec4 vColor;
out vec4 FragColor; //--- ���� ���

void main(void)
{
//FragColor = vec4 (vColor, 1.0);
FragColor = vColor;
}