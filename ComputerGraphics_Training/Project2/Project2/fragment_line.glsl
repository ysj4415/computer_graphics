#version 330 core
//--- out_Color: ���ؽ� ���̴����� �Է¹޴� ���� ��
//--- FragColor: ����� ������ ������ ������ ���۷� ���� ��.


out vec4 FragColor; //--- ���� ���
in vec4 outColor;

void main(void)
{
//FragColor = vec4 (vColor, 1.0);
FragColor = outColor;
}