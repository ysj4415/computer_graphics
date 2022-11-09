#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <stdio.h>
#include <random>
#include <cassert>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
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

//---glsl
void make_vertexShader();
void make_fragmentShader();
void InitShader();
void InitBuffer();
char* filetobuf(const char* file);
GLchar* vertexsource, * fragmentsource;		//---소스코드 저장 변수
GLuint vertexshader, fragmentshader;		//---세이더 객체
GLuint vao, vao_robot, vao_obstacle;
GLuint vbo, vbo_robot, vbo_obstacle;
GLuint ebo, ebo_robot, ebo_obstacle;
//---쉐이더 프로그램
GLuint s_program;


//---애니메이션 bool
bool PlayingOpenBoxAnim = false;


//---각 도형 정보 클래스
class shapestate
{
private:
	glm::vec3 trans;
	glm::vec3 rotate;
	glm::vec3 pibot;
	glm::vec3 color;
public:
	shapestate()
	{
		trans.x = 0.0; trans.y = 0.0; trans.z = 0.0;
		rotate.x = 0.0; rotate.y = 0.0; rotate.z = 0.0;
		pibot.x = 0.0; pibot.y = 0.0; pibot.z = 0.0;
		color.x = 0.0; color.y = 0.0; color.z = 0.0;
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
	//---멤버변수 GET
	glm::vec3 GetTrans() { return trans; }
	glm::vec3 GetRotate() { return rotate; }
	glm::vec3 GetPibot() { return pibot; }
	glm::vec3 GetColor() { return color; }


	glm::mat4 RotateAtPibot(glm::mat4 TR)
	{
		TR = glm::translate(TR, glm::vec3(pibot.x, pibot.y, pibot.z));
		TR = glm::rotate(TR, (GLfloat)glm::radians(rotate.x), glm::vec3(1.0, 0.0, 0.0));
		TR = glm::rotate(TR, (GLfloat)glm::radians(rotate.y), glm::vec3(0.0, 1.0, 0.0));
		TR = glm::rotate(TR, (GLfloat)glm::radians(rotate.z), glm::vec3(0.0, 0.0, 1.0));
		TR = glm::translate(TR, glm::vec3(-1 * pibot.x, -1 * pibot.y, -1 * pibot.z));

		return TR;
	}
	glm::mat4 Translate(glm::mat4 TR)
	{
		TR = glm::translate(TR, glm::vec3(trans.x, trans.y, trans.z));

		return TR;
	}
	bool CrashCheck(GLfloat selfhalfsize, GLfloat selfheight, GLfloat x, GLfloat y, GLfloat z, GLfloat halfsize)
	{

		if (x + halfsize > trans.x - selfhalfsize && x - halfsize < trans.x + selfhalfsize && z + halfsize > trans.z - selfhalfsize && z - halfsize < trans.z + selfhalfsize && y < selfheight)
		{
			return true;
		}
		else
			return false;
	}
};

shapestate boxside[6];
shapestate robot_part[7];
shapestate obstacle[2];
GLfloat obstacle_halfsize = 0.7;
GLfloat obstacle_height = 0.4;

enum robotpart
{
	BODY = 0,
	HEAD,
	LEFTLEG,
	RIGHTLEG,
	RIGHTARM,
	LEFTARM,
	NOSE
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

GLfloat robot_rotate = 90.0f;
glm::vec3 robot_trans = glm::vec3(0.0, 0.0, 0.0);

int robot_dir = 0;		// right


std::random_device rd;
std::default_random_engine eng(rd());
std::uniform_int_distribution<int> rd_count(1, 2);
int obstacle_count = rd_count(rd);

GLfloat jumppower = 0;

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
	boxside[BACK].SetColor(0.7f, 0.7f, 0.7f);
	boxside[FRONT].SetColor(0.8f, 0.8f, 1.0f);
	boxside[RIGHT].SetColor(0.7f, 0.5f, 0.2f);
	boxside[LEFT].SetColor(0.2f, 0.5f, 1.0f);
	boxside[TOP].SetColor(0.8f, 0.8f, 1.0f);
	boxside[BOTTOM].SetColor(1.0f, 1.0f, 1.0f);

	boxside[FRONT].SetPibot(0.0f, 0.0f, 4.0f);

	robot_part[BODY].SetColor(0.0f, 0.0f, 1.0f);
	robot_part[HEAD].SetColor(0.0f, 0.0f, 1.0f);
	robot_part[LEFTLEG].SetColor(1.0f, 1.0f, 0.0f);
	robot_part[RIGHTLEG].SetColor(1.0f, 1.0f, 0.0f);
	robot_part[LEFTARM].SetColor(1.0f, 0.0f, 0.0f);
	robot_part[RIGHTARM].SetColor(0.0f, 1.0f, 0.0f);
	robot_part[NOSE].SetColor(1.0f, 1.0f, 0.0f);

	robot_part[LEFTARM].SetPibot(-0.5f, 0.125f, 0.0f);
	robot_part[RIGHTARM].SetPibot(0.5f, 0.125f, 0.0f);
	robot_part[LEFTLEG].SetPibot(-0.25f, -0.7f, 0.0f);
	robot_part[RIGHTLEG].SetPibot(0.25f, -0.7f, 0.0f);


	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<GLclampf> rd_trans(-3.0, 3.0);
	std::uniform_real_distribution<GLclampf> rd_color(0.0, 1.0);

	do {
		obstacle[0].SetTrans(rd_trans(rd), 0.0, rd_trans(rd));
	} while (obstacle[0].CrashCheck(obstacle_halfsize, obstacle_height, robot_trans.x, robot_trans.y, robot_trans.z, 0.4));
	do {
		obstacle[1].SetTrans(rd_trans(rd), 0.0, rd_trans(rd));
	} while (obstacle[1].CrashCheck(obstacle_halfsize, obstacle_height, obstacle[0].GetTrans().x, obstacle[0].GetTrans().y, obstacle[0].GetTrans().z, obstacle_halfsize) && 
		obstacle[1].CrashCheck(obstacle_halfsize, obstacle_height, robot_trans.x, robot_trans.y, robot_trans.z, 0.4));
	obstacle[0].SetColor(rd_color(rd), rd_color(rd), rd_color(rd));
	obstacle[1].SetColor(rd_color(rd), rd_color(rd), rd_color(rd));

	InitShader();
	InitBuffer();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(10, TimerFunction, 1);

	glutMainLoop();
  }
GLvoid drawScene()
{

	//---배경 초기화
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//---카메라 설정

	glm::vec4 cameraPos_trans(0.0f, 4.0f, 10.0f, 1.0f);
	glm::vec4 cameraDirection_trans(0.0f, 4.0f, 0.0f, 1.0f);

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

	//---------- 육면체 그리기(우)
	glUseProgram(s_program);
	glUniformMatrix4fv(viewLoc_shape, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_shape, 1, GL_FALSE, &projection[0][0]);

	unsigned int modelLoc = glGetUniformLocation(s_program, "model");
	int vColorLocation = glGetUniformLocation(s_program, "vColor");

	glm::mat4 TR_cube = glm::mat4(1.0f);
	glm::mat4 T_cube = glm::mat4(1.0f);
	glm::mat4 Rx_cube = glm::mat4(1.0f);
	glm::mat4 Ry_cube = glm::mat4(1.0f);
	glm::mat4 S_cube = glm::mat4(1.0f);
	glm::mat4 OS_cube = glm::mat4(1.0f);

	T_cube = glm::translate(T_cube, glm::vec3(0.0, 0.0, 0.0));
	Rx_cube = glm::rotate(Rx_cube, (GLfloat)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
	Ry_cube = glm::rotate(Ry_cube, (GLfloat)glm::radians(0.0), glm::vec3(0.0, 1.0, 0.0));
	S_cube = glm::scale(S_cube, glm::vec3(1.0, 1.0, 1.0));

	glBindVertexArray(vao);
	for (int i = 0; i < 6; i++)
	{
		TR_cube = T_cube * Rx_cube * Ry_cube * S_cube;
		TR_cube = boxside[i].Translate(TR_cube);
		TR_cube = boxside[i].RotateAtPibot(TR_cube);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(TR_cube));
		glUniform4f(vColorLocation, boxside[i].GetColor().x, boxside[i].GetColor().y, boxside[i].GetColor().z, 1.0f);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * i * 6));
	}
	//---------- 로봇 그리기
	glUseProgram(s_program);
	glUniformMatrix4fv(viewLoc_shape, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_shape, 1, GL_FALSE, &projection[0][0]);


	glm::mat4 TR_cube_robot = glm::mat4(1.0f);
	glm::mat4 T_cube_robot = glm::mat4(1.0f);
	glm::mat4 Rx_cube_robot = glm::mat4(1.0f);
	glm::mat4 Ry_cube_robot = glm::mat4(1.0f);
	glm::mat4 S_cube_robot = glm::mat4(1.0f);

	T_cube_robot = glm::translate(T_cube_robot, glm::vec3(robot_trans.x, robot_trans.y + 1.5, robot_trans.z));
	Rx_cube_robot = glm::rotate(Rx_cube_robot, (GLfloat)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
	Ry_cube_robot = glm::rotate(Ry_cube_robot, (GLfloat)glm::radians(robot_rotate), glm::vec3(0.0, 1.0, 0.0));

	S_cube_robot = glm::scale(S_cube_robot, glm::vec3(1.0, 1.0, 1.0));

	glBindVertexArray(vao_robot);
	for (int i = 0; i < 7; i++)
	{
		TR_cube_robot = T_cube_robot * Rx_cube_robot * Ry_cube_robot * S_cube_robot;
		TR_cube_robot = robot_part[i].Translate(TR_cube_robot);
		TR_cube_robot = robot_part[i].RotateAtPibot(TR_cube_robot);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(TR_cube_robot));
		glUniform4f(vColorLocation, robot_part[i].GetColor().x, robot_part[i].GetColor().y, robot_part[i].GetColor().z, 1.0f);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * i * 36));
	}
	//---------- 장애물 그리기
	glUseProgram(s_program);
	glUniformMatrix4fv(viewLoc_shape, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_shape, 1, GL_FALSE, &projection[0][0]);


	glm::mat4 TR_cube_obstacle = glm::mat4(1.0f);
	glm::mat4 T_cube_obstacle = glm::mat4(1.0f);
	glm::mat4 Rx_cube_obstacle = glm::mat4(1.0f);
	glm::mat4 Ry_cube_obstacle = glm::mat4(1.0f);
	glm::mat4 S_cube_obstacle = glm::mat4(1.0f);

	T_cube_obstacle = glm::translate(T_cube_obstacle, glm::vec3(0.0, 0.0, 0.0));
	Rx_cube_obstacle = glm::rotate(Rx_cube_obstacle, (GLfloat)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
	Ry_cube_obstacle = glm::rotate(Ry_cube_obstacle, (GLfloat)glm::radians(0.0), glm::vec3(0.0, 1.0, 0.0));

	S_cube_obstacle = glm::scale(S_cube_obstacle, glm::vec3(1.0, 1.0, 1.0));

	glBindVertexArray(vao_obstacle);
	for (int i = 0; i < obstacle_count; i++)
	{
		TR_cube_obstacle = T_cube_obstacle * Rx_cube_obstacle * Ry_cube_obstacle * S_cube_obstacle;
		TR_cube_obstacle = obstacle[i].Translate(TR_cube_obstacle);
		TR_cube_obstacle = obstacle[i].RotateAtPibot(TR_cube_obstacle);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(TR_cube_obstacle));
		glUniform4f(vColorLocation, obstacle[i].GetColor().x, obstacle[i].GetColor().y, obstacle[i].GetColor().z, 1.0f);

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 0));
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

	obj objfile("cube_20.obj");
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

	//----------------로봇

	obj objfile_robot("robot.obj");
	objfile_robot.ReadObj();
	glGenVertexArrays(1, &vao_robot);
	glGenBuffers(1, &vbo_robot);
	glGenBuffers(1, &ebo_robot);
	glBindVertexArray(vao_robot);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_robot);
	glBufferData(GL_ARRAY_BUFFER, objfile_robot.vertexNum * 3 * sizeof(GLfloat), objfile_robot.vertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_robot);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, objfile_robot.faceNum * 3 * sizeof(int), objfile_robot.face_v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);

	//----------------장애물

	obj objfile_obstacle("obstacle.obj");
	objfile_obstacle.ReadObj();
	glGenVertexArrays(1, &vao_obstacle);
	glGenBuffers(1, &vbo_obstacle);
	glGenBuffers(1, &ebo_obstacle);
	glBindVertexArray(vao_obstacle);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_obstacle);
	glBufferData(GL_ARRAY_BUFFER, objfile_obstacle.vertexNum * 3 * sizeof(GLfloat), objfile_obstacle.vertex, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_obstacle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, objfile_obstacle.faceNum * 3 * sizeof(int), objfile_obstacle.face_v, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
}


GLfloat arm_swip = 3.0;
GLfloat gravity = 0.8;

GLvoid TimerFunction(int value)
{
	if (value == 0 && PlayingOpenBoxAnim == true)
	{
		boxside[FRONT].PlusRotate(0.5, 0.0, 0.0);

		if (boxside[FRONT].GetRotate().x >= 90)
		{
			boxside[FRONT].SetRotate(90, 0, 0);
			PlayingOpenBoxAnim = false;
		}
		glutTimerFunc(10, TimerFunction, 0);
	}
	if (value == 1)
	{
		robot_part[LEFTARM].PlusRotate(arm_swip, 0.0, 0.0);
		robot_part[RIGHTARM].PlusRotate(-1 * arm_swip, 0.0, 0.0);

		robot_part[LEFTLEG].PlusRotate(-1 * arm_swip, 0.0, 0.0);
		robot_part[RIGHTLEG].PlusRotate(arm_swip, 0.0, 0.0);

		if (robot_part[LEFTARM].GetRotate().x <= -45 || robot_part[LEFTARM].GetRotate().x >= 45)
			arm_swip *= -1;
		int count = 0;
		switch (robot_dir)
		{
		case 0:
			count = 0;
			for (int i = 0; i < obstacle_count; i++)
			{
				if (obstacle[i].CrashCheck(obstacle_halfsize, obstacle_height, robot_trans.x + 0.07f, robot_trans.y, robot_trans.z, 0.4))
					count++;
			}
			if(count == 0)
				robot_trans.x = (robot_trans.x + 0.07f);
			break;
		case 1:
			count = 0;
			for (int i = 0; i < obstacle_count; i++)
			{
				if (obstacle[i].CrashCheck(obstacle_halfsize, obstacle_height, robot_trans.x - 0.07f, robot_trans.y, robot_trans.z, 0.4))
					count++;
			}
			if (count == 0)
				robot_trans.x = (robot_trans.x - 0.07f);

			break;
		case 2:
			count = 0;
			for (int i = 0; i < obstacle_count; i++)
			{
				if (obstacle[i].CrashCheck(obstacle_halfsize, obstacle_height, robot_trans.x, robot_trans.y, robot_trans.z - 0.07f, 0.4))
					count++;
			}
			if (count == 0)
					robot_trans.z = (robot_trans.z - 0.07f);

			break;
		case 3:
			count = 0;
			for (int i = 0; i < obstacle_count; i++)
			{
				if (obstacle[i].CrashCheck(obstacle_halfsize, obstacle_height, robot_trans.x, robot_trans.y, robot_trans.z + 0.07f, 0.4))
					count++;
			}
			if (count == 0)
					robot_trans.z = (robot_trans.z + 0.07f);
			break;
		}

		if (robot_trans.x >= 4.0f)
			robot_trans.x = -3.9f;
		else if(robot_trans.x <= -4.0f)
			robot_trans.x = 3.9f;
		if (robot_trans.z >= 4.0f)
			robot_trans.z = -3.9f;
		else if (robot_trans.z <= -4.0f)
			robot_trans.z = 3.9f;


		if (jumppower > 0)
		{
			robot_trans.y += jumppower;
			jumppower -= 0.1;
		}
		else if (jumppower < 0)
			jumppower = 0;


		robot_trans.y -= gravity;
		
		count = 0;
		for (int i = 0; i < obstacle_count; i++)
		{
			if (obstacle[i].CrashCheck(obstacle_halfsize, obstacle_height, robot_trans.x, robot_trans.y, robot_trans.z, 0.4))
				count++;
		}
		if (robot_trans.y < 0)
			robot_trans.y = 0;
		if (count != 0 && robot_trans.y < obstacle_height)
			robot_trans.y = obstacle_height;

		glutTimerFunc(10, TimerFunction, 1);
	}
	
	glutPostRedisplay();
}



GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'o':
		if (PlayingOpenBoxAnim == false)
		{
			PlayingOpenBoxAnim = true;
			glutTimerFunc(10, TimerFunction, 0);
		}
		break;
	case 'd':
		robot_rotate = 90;
		robot_dir = 0;
		break;
	case 'a':
		robot_rotate = 270;
		robot_dir = 1;
		break;
	case 'w':
		robot_rotate = 180;
		robot_dir = 2;
		break;
	case 's':
		robot_rotate = 0;
		robot_dir = 3;
		break;
	case 'j':
		if (jumppower == 0)
			jumppower = 1.5;
		break;
	case 'i':
		boxside[FRONT].SetRotate(90, 0, 0);
		PlayingOpenBoxAnim = false;
		robot_rotate = 90;
		robot_dir = 0;
		robot_trans = glm::vec3(0.0, 0.0, 0.0);
		break;
	case 'z':
		if (camera_trans.z > -3.2)
			camera_trans.z -= 0.2;
		break;
	case 'Z':
		if (camera_trans.z < 4.0)
			camera_trans.z += 0.2;
		break;
	case 'x':
		if (camera_trans.x > -4.0)
			camera_trans.x -= 0.2;
		break;
	case 'X':
		if (camera_trans.x < 4.0)
			camera_trans.x += 0.2;
		break;
	case 'y':
		if (rotate_screen > -90)
			rotate_screen -= 1.0;
		break;
	case 'Y':
		if (rotate_screen < 90)
			rotate_screen += 1.0;
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
