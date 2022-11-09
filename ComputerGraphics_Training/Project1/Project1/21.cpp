#define _CRT_SECURE_NO_WARNINGS
#include <random>
#include <cassert>
#include "objReader.h"
#include <cmath>

using namespace std;

//---윈도우 사이즈 변수
int WinSize_w = 800;
int WinSize_h = 800;
int windowID;		//---윈도우 아이디

//---콜백 함수
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid TimerFunction(int value);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Motion(int x, int y);

//---glsl
void make_vertexShader();
void make_fragmentShader();
void InitShader();
void InitBuffer();
char* filetobuf(const char* file);
GLchar* vertexsource, * fragmentsource;		//---소스코드 저장 변수
GLuint vertexshader, fragmentshader;		//---세이더 객체
GLuint vao, vao_box;
GLuint vbo, vbo_box;
GLuint ebo, ebo_box;
//---쉐이더 프로그램
GLuint s_program;


//---구 모델
GLUquadricObj* qobj;
GLfloat radius = 0.2;
//---각 도형 정보 클래스
class shapestate
{
private:
	glm::vec3 trans;
	glm::vec3 rotate;
	glm::vec3 pibot;
	glm::vec3 color;
	glm::vec3 nomal;
public:
	glm::vec3* vertex = NULL;
	shapestate()
	{
		trans.x = 0.0; trans.y = 0.0; trans.z = 0.0;
		rotate.x = 0.0; rotate.y = 0.0; rotate.z = 0.0;
		pibot.x = 0.0; pibot.y = 0.0; pibot.z = 0.0;
		color.x = 0.0; color.y = 0.0; color.z = 0.0;
		nomal.x = 0.0; nomal.y = 0.0; nomal.z = 0.0;
	}
	shapestate(glm::vec3 p, glm::vec3 c)
	{
		trans.x = 0.0; trans.y = 0.0; trans.z = 0.0;
		rotate.x = 0.0; rotate.y = 0.0; rotate.z = 0.0;
		pibot = p;
		color = c;
	}
	//---멤버변수 SET
	void PlusTrans(GLfloat x, GLfloat y, GLfloat z)
	{
		trans.x += x; trans.y += y; trans.z += z;
	}
	void SetTrans(GLfloat x, GLfloat y, GLfloat z)
	{
		trans.x = x; trans.y = y; trans.z = z;
	}
	void PlusRotate(GLfloat x, GLfloat y, GLfloat z)
	{
		rotate.x += x; rotate.y += y; rotate.z += z;
	}
	void SetRotate(GLfloat x, GLfloat y, GLfloat z)
	{
		rotate.x = x; rotate.y = y; rotate.z = z;
	}
	void SetPibot(GLfloat x, GLfloat y, GLfloat z)
	{
		pibot.x = x; pibot.y = y; pibot.z = z;
	}
	void SetColor(GLfloat x, GLfloat y, GLfloat z)
	{
		color.x = x; color.y = y; color.z = z;
	}
	void SetNomal(GLfloat x, GLfloat y, GLfloat z)
	{
		nomal.x = x; nomal.y = y; nomal.z = z;
	}
	//---멤버변수 GET
	glm::vec3 GetTrans() { return trans; }
	glm::vec3 GetRotate() { return rotate; }
	glm::vec3 GetPibot() { return pibot; }
	glm::vec3 GetColor() { return color; }
	glm::vec3 GetNomal() { return nomal; }

	glm::mat4 Translate(glm::mat4 TR)
	{
		TR = glm::translate(TR, glm::vec3(trans.x, trans.y, trans.z));

		return TR;
	}

};
enum BoxSide
{
	BACK = 0,
	LEFT = 1,
	TOP = 2,
	RIGHT = 3,
	BOTTOM = 4,
	FRONT = 5
};
class cube
{
private:
	glm::vec3* vertex;
	glm::vec3 rotate;


public:
	shapestate side[6];

	cube() { 
		vertex = NULL; 
		rotate = glm::vec3(0.0,0.0,0.0);
	}
	void SetVertex(glm::vec3* v)
	{
		vertex = v;
	}
	int RangeCheck_Point(GLfloat x, GLfloat y, GLfloat z)
	{
		if (GetDis(0,LEFT, x, y, z) < radius)
			return LEFT;
		else if (GetDis(4, RIGHT, x, y, z) < radius)
			return RIGHT;
		else if (GetDis(0, BOTTOM, x, y, z) < radius)
			return BOTTOM;
		else if (GetDis(2, TOP, x, y, z) < radius)
			return TOP;
		else if (GetDis(0, BACK, x, y, z) < radius)
			return BACK;
		else if (GetDis(1, FRONT, x, y, z) < radius)
			return FRONT;
		else return -1;


	}
	GLfloat GetDis(int index, int sidenum, GLfloat x, GLfloat y, GLfloat z)
	{
		glm::vec4 v = glm::vec4(vertex[index].x, vertex[index].y, vertex[index].z, 1.0f);
		glm::vec4 n = glm::vec4(side[sidenum].GetNomal().x, side[sidenum].GetNomal().y, side[sidenum].GetNomal().z, 1.0f);

		glm::mat4 rot = glm::mat4(1.0f);
		rot = glm::rotate(rot, (GLfloat)glm::radians(rotate.z), glm::vec3(0.0, 0.0, 1.0));
		v = rot * v;
		n = rot * n;

		GLfloat a = n.x;
		GLfloat b = n.y;
		GLfloat c = n.z;
		GLfloat d = -1 * (v.x * n.x + v.y * n.y + v.z * n.z);
		return abs(a * x + b * y + c * z + d) / sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2));
	}
	glm::vec3 GetRotate() { return rotate; }
	void SetRotate(GLfloat x, GLfloat y, GLfloat z) { rotate = glm::vec3(x, y, z); }

	int RangeCheck_box(GLfloat x, GLfloat y, GLfloat z)
	{
		GLfloat underpoint[8] = { 0 };
		for (int i = 0; i < 8; i++)
		{
			glm::vec4 v = glm::vec4(vertex[i].x, vertex[i].y, vertex[i].z, 1.0f);

			glm::mat4 rot = glm::mat4(1.0f);
			rot = glm::rotate(rot, (GLfloat)glm::radians(rotate.z), glm::vec3(0.0, 0.0, 1.0));
			v = rot * v;
		}

	}
};

cube box;
cube smallbox;
shapestate orb[5];
int orbcount = 1;





glm::vec3 camera_trans(0.0f, 0.0f, 0.0f);
GLfloat rotate_screen = 0.0;

void main(int argc, char** argv)		//---윈도우 출력, 콜백함수 설정
{
	//---윈도우 생성
	glutInit(&argc, argv);		//glut초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);	//디스플레이 모드 설정
	glutInitWindowPosition(0, 0);					//윈도우 위치 지정
	glutInitWindowSize(WinSize_w, WinSize_h);					//윈도우 크기 지정
	windowID = glutCreateWindow("Example1");					//윈도우 생성

	//---GLEW 초기화
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)		//glew 초기화에 실패할 경우
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW initialized\n";

	//glEnable(GL_DEPTH_TEST);
	box.side[BACK].SetColor(0.8f, 0.8f, 0.8f);
	box.side[RIGHT].SetColor(0.35f, 0.35f, 0.35f);
	box.side[LEFT].SetColor(0.4f, 0.4f, 0.4f);
	box.side[TOP].SetColor(0.65f, 0.65f, 0.65f);
	box.side[BOTTOM].SetColor(0.6f, 0.6f, 0.6f);

	box.side[BACK].SetNomal(0.0f, 0.0f, 1.0f);
	box.side[RIGHT].SetNomal(-1.0f, 0.0f, 0.0f);
	box.side[LEFT].SetNomal(1.0f, 0.0f, 0.0f);
	box.side[TOP].SetNomal(0.0f, -1.0f, 0.0f);
	box.side[BOTTOM].SetNomal(0.0f, 1.0f, 0.0f);
	box.side[FRONT].SetNomal(0.0f, 0.0f, -1.0f);


	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<GLclampf> rd_nomal(-0.05, +0.05);
	for (int i = 0; i < 5; i++)
	{
		orb[i].SetNomal(rd_nomal(rd), rd_nomal(rd), rd_nomal(rd));
	}


	qobj = gluNewQuadric(); // 객체 생성하기
	gluQuadricDrawStyle(qobj, GLU_FILL); // 도형 스타일
	gluQuadricNormals(qobj, GLU_SMOOTH); // 생략 가능
	gluQuadricOrientation(qobj, GLU_OUTSIDE);

	glEnable(GL_DEPTH_TEST);

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);

	glutTimerFunc(10, TimerFunction, 0);

	glutMainLoop();
   }
GLvoid drawScene()
{

	//---배경 초기화
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//---카메라 설정

	glm::vec4 cameraPos_trans(0.0f, 0.0f, 10.0f, 1.0f);
	glm::vec4 cameraDirection_trans(0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 pos = glm::mat4(1.0f);
	pos = glm::rotate(pos, (GLfloat)glm::radians(rotate_screen), glm::vec3(0.0, 1.0, 0.0));
	pos = glm::translate(pos, glm::vec3(camera_trans.x, camera_trans.y, camera_trans.z));
	cameraPos_trans = pos * cameraPos_trans;

	cameraDirection_trans = pos * cameraDirection_trans;


	glm::vec3 cameraPos = glm::vec3(cameraPos_trans.x, cameraPos_trans.y, cameraPos_trans.z);		 //--- 카메라 위치
	glm::vec3 cameraDirection = glm::vec3(cameraDirection_trans.x, cameraDirection_trans.y, cameraDirection_trans.z); //--- 카메라 바라보는 방향
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);		 //--- 카메라 위쪽 방향
	glm::mat4 view = glm::mat4(1.0f);

	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	unsigned int viewLoc_shape = glGetUniformLocation(s_program, "view"); //--- 뷰잉 변환 설정

	//---투영변환
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, -5.0)); //--- 공간을 약간 뒤로 미뤄줌


	unsigned int projLoc_shape = glGetUniformLocation(s_program, "projection");

	//---------- 육면체 그리기
	glUseProgram(s_program);
	glUniformMatrix4fv(viewLoc_shape, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_shape, 1, GL_FALSE, &projection[0][0]);

	unsigned int modelLoc = glGetUniformLocation(s_program, "model");
	int vColorLocation = glGetUniformLocation(s_program, "vColor");

	glm::mat4 TR_cube = glm::mat4(1.0f);
	glm::mat4 T_cube = glm::mat4(1.0f);
	glm::mat4 Rx_cube = glm::mat4(1.0f);
	glm::mat4 Rz_cube = glm::mat4(1.0f);
	glm::mat4 S_cube = glm::mat4(1.0f);

	T_cube = glm::translate(T_cube, glm::vec3(0.0, 0.0, 0.0));
	Rx_cube = glm::rotate(Rx_cube, (GLfloat)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
	Rz_cube = glm::rotate(Rz_cube, (GLfloat)glm::radians(box.GetRotate().z), glm::vec3(0.0, 0.0, 1.0));
	S_cube = glm::scale(S_cube, glm::vec3(1.0, 1.0, 1.0));

	glBindVertexArray(vao);
	for (int i = 0; i < 5; i++)
	{
		TR_cube = T_cube * Rx_cube * Rz_cube * S_cube;
		TR_cube = box.side[i].Translate(TR_cube);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(TR_cube));
		glUniform4f(vColorLocation, box.side[i].GetColor().x, box.side[i].GetColor().y, box.side[i].GetColor().z, 1.0f);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * i * 6));
	}
	
	//---------- 구 그리기
	glUseProgram(s_program);
	glUniformMatrix4fv(viewLoc_shape, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_shape, 1, GL_FALSE, &projection[0][0]);

	glm::mat4 TR_orb = glm::mat4(1.0f);
	glm::mat4 T_orb = glm::mat4(1.0f);
	glm::mat4 Rx_orb = glm::mat4(1.0f);
	glm::mat4 Ry_orb = glm::mat4(1.0f);
	glm::mat4 S_orb = glm::mat4(1.0f);

	T_orb = glm::translate(T_orb, glm::vec3(0.0, 0.0, 0.0));
	Rx_orb = glm::rotate(Rx_orb, (GLfloat)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
	Ry_orb = glm::rotate(Ry_orb, (GLfloat)glm::radians(0.0), glm::vec3(0.0, 0.0, 1.0));
	S_orb = glm::scale(S_orb, glm::vec3(1.0, 1.0, 1.0));

	for (int i = 0; i < orbcount; i++)
	{
		TR_orb = T_orb * Rx_orb * Ry_orb * S_orb;
		TR_orb = orb[i].Translate(TR_orb);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(TR_orb));
		glUniform4f(vColorLocation, 0.0, 0.0, 1.0, 1.0f);

		gluSphere(qobj, radius, 50, 50);
	}

	glBindVertexArray(0);



	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);

}

void make_vertexShader()
{
	vertexsource = filetobuf("vertex_shape_sidecolor.glsl");


	//--- 버텍스 세이더 객체 만들기
	vertexshader = glCreateShader(GL_VERTEX_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);

	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexshader);

	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexshader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
	
}
void make_fragmentShader()
{
	fragmentsource = filetobuf("fragment_shape_sidecolor.glsl");


	//--- 프래그먼트 세이더 객체 만들기
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentshader);

	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- 버텍스 세이더 만들기
	make_fragmentShader(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	s_program = glCreateProgram();

	glAttachShader(s_program, vertexshader);
	glAttachShader(s_program, fragmentshader);

	//--- 세이더 삭제하기

	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	

	glLinkProgram(s_program);

	// ---세이더가 잘 연결되었는지 체크하기
	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(s_program, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(s_program, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return;
	}
}

void InitBuffer()
{
	//----------------육면체

	obj objfile("cube_21.obj");
	objfile.ReadObj();
	glGenVertexArrays(1,& vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, objfile.vertexNum * 3 * sizeof(GLfloat), objfile.vertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, objfile.faceNum * 3 * sizeof(int), objfile.face_v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	box.SetVertex(objfile.vertex);

	//----------------상자

	obj objfile_box("box_21.obj");
	objfile_box.ReadObj();
	glGenVertexArrays(1, &vao_box);
	glGenBuffers(1, &vbo_box);
	glGenBuffers(1, &ebo_box);
	glBindVertexArray(vao_box);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_box);
	glBufferData(GL_ARRAY_BUFFER, objfile_box.vertexNum * 3 * sizeof(GLfloat), objfile_box.vertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_box);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, objfile_box.faceNum * 3 * sizeof(int), objfile_box.face_v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	smallbox.SetVertex(objfile_box.vertex);
}

int mouseclick_x, mouseclick_y;
bool isLEFT_DOWN = false;
GLfloat radian = 0;
GLfloat firstradian = 0;

GLvoid Mouse(int button, int state, int x, int y)
{

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		isLEFT_DOWN = true;
		mouseclick_x = x;
		mouseclick_y = y;
		firstradian = box.GetRotate().z;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		isLEFT_DOWN = false;
	}
	glutPostRedisplay();
}
GLvoid Motion(int x, int y)
{
	if (isLEFT_DOWN == true)
	{
		float u_x = mouseclick_x - WinSize_h / 2;
		float u_y = mouseclick_y - WinSize_w / 2;
		float v_x = x - WinSize_h / 2;
		float v_y = y - WinSize_w / 2;
		if(firstradian + radian * 180 / 3.141592 < 60 && firstradian + radian * 180 / 3.141592 > -60)
			radian = atan(u_y / u_x) - atan(v_y / v_x);

		box.SetRotate(0, 0, firstradian + radian * 180 / 3.141592);
		cout << radian * 180 / 3.141592<< endl;
	}
	glutPostRedisplay();
}
GLvoid TimerFunction(int value)
{
	if (value == 0)
	{
		for (int i = 0; i < orbcount; i++)
		{
			int sidenum = box.RangeCheck_Point(orb[i].GetTrans().x, orb[i].GetTrans().y, orb[i].GetTrans().z);
			if(sidenum == -1)
				orb[i].PlusTrans(orb[i].GetNomal().x, orb[i].GetNomal().y, orb[i].GetNomal().z);
			else if (sidenum != -1)
			{

				glm::vec4 n;
				glm::mat4 rot;
				while (box.RangeCheck_Point(orb[i].GetTrans().x, orb[i].GetTrans().y, orb[i].GetTrans().z) != -1)
				{
					n = glm::vec4(box.side[sidenum].GetNomal().x, box.side[sidenum].GetNomal().y, box.side[sidenum].GetNomal().z, 1.0f);

					rot = glm::mat4(1.0f);
					rot = glm::rotate(rot, (GLfloat)glm::radians(box.GetRotate().z), glm::vec3(0.0, 0.0, 1.0));
					n = rot * n;
					orb[i].PlusTrans(n.x * 0.1, n.y * 0.1, n.z * 0.1);
				}

				orb[i].SetNomal(n.x * 0.1, n.y * 0.1, n.z * 0.1);
			}

		}
		glutTimerFunc(1, TimerFunction, 0);
	}
	
	glutPostRedisplay();
}



GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'b':
		if (orbcount < 5)
			orbcount++;
		break;
	case 'q':
		glutDestroyWindow(windowID);
		break;
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
