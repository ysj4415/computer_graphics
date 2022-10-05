#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <cassert>
#define CheckBox 0.06
class Point;


//---������ ������ ����
int WinSize_r = 1200;
int WinSize_w = 800;
GLfloat changeratio = (GLfloat)WinSize_r / WinSize_w;
using namespace std;
int windowID;		//---������ ���̵�

//---�ݹ� �Լ�
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

//------����� ���� �Լ�
GLfloat* MakeArray(Point[4]);


//---glsl
void make_vertexShader();
void make_fragmentShader();
void InitShader();
void InitBuffer();
char* filetobuf(const char* file);
GLchar* vertexsource, * fragmentsource;		//---�ҽ��ڵ� ���� ����
GLuint vertexshader, fragmentshader;		//---���̴� ��ü
GLuint vao_line, vao;
GLuint vbo_line, vbo;
GLuint ebo;
//---���̴� ���α׷�
GLuint s_program;

//---��
GLfloat back_line[12] = { -1.0 , 0, 0,
						1.0, 0, 0, 
						0, 1.0, 0, 
						0, -1.0 };

GLfloat* vertex = NULL;
int vertex_ebo[] = { 0,1,  1,2,  2,3,  3,0 };
int MouseClick = -1;
bool InRect = false;
GLfloat ori_x, ori_y;
class Point
{
private:
	GLfloat x, y;
	int quadrant;
public:
	Point() : x(0), y(0), quadrant(0) {}
	~Point() {}
	int SetQuadrant()
	{
		if (x > 0 && y >= 0)
			quadrant = 1;
		else if (x <= 0 && y > 0)
			quadrant = 2;
		else if (x < 0 && y <= 0)
			quadrant = 3;
		else if (x >= 0 && y < 0)
			quadrant = 4;
		return quadrant;
	}
	void SetPoint(GLfloat a, GLfloat b) { x = a; y = b; SetQuadrant(); }
	bool CheckMouse(int a, int b)
	{
		GLfloat mx = ((GLfloat)(a - (WinSize_r / 2)) / (WinSize_r / 2));
		GLfloat my = ((GLfloat)((WinSize_w / 2) - b) / (WinSize_w / 2));

		if (mx >= x - CheckBox && mx <= x + CheckBox && my >= y - CheckBox && my <= y + CheckBox)
			return true;
		else
			return false;
	}
	void MovePoint(int a, int b)
	{
		GLfloat mx = ((GLfloat)(a - (WinSize_r / 2)) / (WinSize_r / 2));
		GLfloat my = ((GLfloat)((WinSize_w / 2) - b) / (WinSize_w / 2));
		SetPoint(mx, my);
	}
	GLfloat GetX() { return x; }
	GLfloat GetY() { return y; }
};
GLfloat* MakeArray(Point p[4])
{
	GLfloat* arr = new GLfloat[12]{ 0 };
	for (int i = 0; i < 4; i++)
	{
		arr[i * 3] = p[i].GetX();
		arr[i * 3 + 1] = p[i].GetY();
		arr[i * 3 + 2] = 0;
	}

	return arr;
}
Point spot[4];
Point ori_spot[4];

bool CheckInRect(int click_x, int click_y)
{
	GLfloat x = ((GLfloat)(click_x - (WinSize_r / 2)) / (WinSize_r / 2));
	GLfloat y = ((GLfloat)((WinSize_w / 2) - click_y) / (WinSize_w / 2));

	GLfloat range[4];
	for (int i = 0; i < 4; i++)
	{
		GLfloat a;
		if (i % 2 == 0)
		{
			a = (spot[i].GetY() - spot[(i + 1) % 4].GetY()) / (spot[i].GetX() - spot[(i + 1) % 4].GetX());
			range[i] = a * (x - spot[i + 1].GetX()) + spot[i].GetY();
		}
		else
		{
			a = (spot[i].GetX() - spot[(i + 1) % 4].GetX()) / (spot[i].GetY() - spot[(i + 1) % 4].GetY());
			range[i] = a * (y - spot[i].GetY()) + spot[i].GetX();
		}
	}
	cout << endl;
	if (y < range[0] && y > range[2] && x > range[1] && x < range[3])
		return true;
	else
		return false;
}

float GetArea(Point p[], int count)
{
	float det_sum = 0;
	float i_det_sum = 0;

	for (int i = 0; i < count; i++)
	{
		int i_x = (WinSize_r / 2) * p[i].GetX() + (WinSize_r / 2);
		int i_y = (WinSize_w / 2) - (WinSize_w / 2) * p[i].GetY();
		int i_x2 = (WinSize_r / 2) * p[(i + 1) % count].GetX() + (WinSize_r / 2);
		int i_y2 = (WinSize_w / 2) - (WinSize_w / 2) * p[(i + 1) % count].GetY();
		GLfloat x = p[i].GetX();
		GLfloat y =p[i].GetY();
		GLfloat x2 = p[(i + 1) % count].GetX();
		GLfloat y2 = p[(i + 1) % count].GetY();
		det_sum += x * y2 - x2 * y;
	}
	return fabs(det_sum) / 2;
}
float i_GetArea(Point p[], int count)
{
	float det_sum = 0;

	for (int i = 0; i < count; i++)
	{
		int x = (WinSize_r / 2) * p[i].GetX() + (WinSize_r / 2);
		int y = (WinSize_w / 2) - (WinSize_w / 2) * p[i].GetY();
		int x2 = (WinSize_r / 2) * p[(i + 1) % count].GetX() + (WinSize_r / 2);
		int y2 = (WinSize_w / 2) - (WinSize_w / 2) * p[(i + 1) % count].GetY();
		det_sum += x * y2 - x2 * y;
	}
	return fabs(det_sum) / 2;
}
void main(int argc, char** argv)		//---������ ���, �ݹ��Լ� ����
{
	//---������ ����
	glutInit(&argc, argv);		//glut�ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);	//���÷��� ��� ����
	glutInitWindowPosition(0, 0);					//������ ��ġ ����
	glutInitWindowSize(WinSize_r, WinSize_w);					//������ ũ�� ����
	windowID = glutCreateWindow("Example1");					//������ ����

	//---GLEW �ʱ�ȭ
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		//glew �ʱ�ȭ�� ������ ���
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW initialized\n";
	
	
	spot[0].SetPoint(0.5, 0.5);
	spot[1].SetPoint(-0.5, 0.5);
	spot[2].SetPoint(-0.5, -0.5);
	spot[3].SetPoint(0.5, -0.5);


	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMotionFunc(Motion);
	glutMouseFunc(Mouse);


	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	int vColorLocation = glGetUniformLocation(s_program, "vColor");

	glUseProgram(s_program);

	//-------------�� �׸���
	glUniform4f(vColorLocation, 0.0f, 0.0f, 0.0f, 1.0f);


	glBindVertexArray(vao_line);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
		


	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
		
	glUniform4f(vColorLocation, 0.0f, 0.0f, 1.0f, 1.0f);

	glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);

	
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);

}

void make_vertexShader()
{
	vertexsource = filetobuf("vertex.glsl");

	//--- ���ؽ� ���̴� ��ü �����
	vertexshader = glCreateShader(GL_VERTEX_SHADER);

	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);

	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexshader);

	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexshader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}
void make_fragmentShader()
{
	fragmentsource = filetobuf("fragment.glsl");

	//--- �����׸�Ʈ ���̴� ��ü �����
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

	//--- �����׸�Ʈ ���̴� ������
	glCompileShader(fragmentshader);

	//--- �������� ����� ���� ���� ���: ������ ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- ���ؽ� ���̴� �����
	make_fragmentShader(); //--- �����׸�Ʈ ���̴� �����
	//-- shader Program
	s_program = glCreateProgram();

	glAttachShader(s_program, vertexshader);
	glAttachShader(s_program, fragmentshader);

	//--- ���̴� �����ϱ�
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);

	glLinkProgram(s_program);

	// ---���̴��� �� ����Ǿ����� üũ�ϱ�
	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(s_program, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(s_program, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return;
	}
	//--- Shader Program ����ϱ�
}

void InitBuffer()
{
	//--------------�� �׸���
	glGenVertexArrays(1, &vao_line);
	glGenBuffers(1, &vbo_line);
	glBindVertexArray(vao_line);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_line);

	glBufferData(GL_ARRAY_BUFFER,  12 * sizeof(GLfloat), back_line, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);

	if (vertex != NULL) delete vertex;
	vertex = MakeArray(spot);
	//----------------����
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertex, GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertex_ebo, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
}

void RefreshBuffer()
{
	glBindVertexArray(vao);
	if (vertex != NULL) delete vertex;
	vertex = MakeArray(spot);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		glutDestroyWindow(windowID);
		break;
	}
	glutPostRedisplay();
}
GLvoid Mouse(int button, int state, int x, int y)
{
	int ClickNum = 0;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		for (int i = 0; i < 4; i++)
		{
			if (spot[i].CheckMouse(x, y)) MouseClick = i;
		}
		if (MouseClick == -1 && CheckInRect(x, y))
		{
			InRect = true;
			ori_x = x; ori_y = y;
			for (int i = 0; i < 4; i++)
			{
				ori_spot[i].SetPoint(spot[i].GetX(), spot[i].GetY());
			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		if(MouseClick != -1)
			MouseClick = -1;
		if (InRect == true)
			InRect = false;
	}

	glutPostRedisplay();
}
GLvoid Motion(int x, int y)
{
	if (MouseClick != -1)
	{
		spot[MouseClick].MovePoint(x, y);
		RefreshBuffer();
		cout << "Rect Area(GLfloat): " << GetArea(spot, 4) << " / Rect Area(int): " << i_GetArea(spot, 4) << endl;
	}
	else if (InRect == true)
	{
		GLfloat mx = ((GLfloat)(x - (WinSize_r / 2)) / (WinSize_r / 2));
		GLfloat my = ((GLfloat)((WinSize_w / 2) - y) / (WinSize_w / 2));
		GLfloat ori_mx = ((GLfloat)(ori_x - (WinSize_r / 2)) / (WinSize_r / 2));
		GLfloat ori_my = ((GLfloat)((WinSize_w / 2) - ori_y) / (WinSize_w / 2));
		for (int i = 0; i < 4; i++)
		{
			spot[i].SetPoint(ori_spot[i].GetX() + (mx - ori_mx), ori_spot[i].GetY() + (my - ori_my));
		}
		RefreshBuffer();

	}

	glutPostRedisplay();
}
char* filetobuf(const char* file)
{
	FILE* fp;
	long length;
	char* buf;

	fp = fopen(file, "rb");

	if (!fp)
		assert(fp != nullptr);

	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	buf = (char*)malloc(sizeof(char) * length + 1);
	fseek(fp, 0, SEEK_SET);
	if (buf != nullptr)
		fread(buf, 1, length + 1, fp);

	fclose(fp);
	if (buf != nullptr)
		buf[length] = 0;

	return buf;
}