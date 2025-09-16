#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>


std::random_device rd;

// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::mt19937 gen(rd());

// 0 부터 99 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_int_distribution<int> dis(0, 256);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);

bool timerRunning = false;

GLdouble Rvalue = 0.0;
GLdouble Gvalue = 0.0;
GLdouble Bvalue = 1.0;


// 타이머 콜백 (glutTimerFunc 등에서 호출될 함수 예시)
void TimerFunc(int value) {
    if (!timerRunning) return;

    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        glutPostRedisplay();
        glutTimerFunc(1000, TimerFunc, 0); // 1초마다 갱신
    }
}


void main(int argc, char** argv)
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Example1");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(Rvalue, Gvalue, Bvalue, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);


	int x1, y1, x2, y2;
    x1 = 0;
	y1 = 0;
	x2 = 250;
	y2 = 250;


    glRectf((x1 - 250) / 250, (y1 - 250) / -250, 0, 0);

	// 그리기 부분 구현: 그리기 관련 부분이 여기에포함된다.
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	//--- 필요한 헤더파일 include
	//--- 윈도우 출력하고 콜백함수 설정
   // glut 초기화
   // 디스플레이 모드 설정
   // 윈도우의 위치 지정
   // 윈도우의 크기 지정
   // 윈도우 생성 (윈도우 이름)
	// glew 초기화
   // 출력 함수의 지정
   // 다시 그리기 함수 지정
   // 이벤트 처리 시작
   //--- 콜백 함수: 출력 콜백 함수
   // 바탕색을 ‘blue’ 로 지정
   // 설정된 색으로 전체를칠하기
   // 화면에 출력하기
   //--- 콜백 함수: 다시 그리기 콜백 함수
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c': // 청록색 (초록+파랑)
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'm': // 자홍색 (빨강+파랑)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 0.0;
        Bvalue = 1.0;
        break;

    case 'y': // 노랑색 (빨강+초록)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 0.0;
        break;

    case 'a': // 랜덤색
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        timerRunning = false;
        break;

    case 'w': // 백색
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'k': // 검정색
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 0.0;
        Bvalue = 0.0;
        break;

    case 't': // 타이머 시작
        if (!timerRunning) {
            timerRunning = true;
            glutTimerFunc(100, TimerFunc, 0); // 1초마다 랜덤 색 갱신
        }
        break;

    case 's': // 타이머 종료
        timerRunning = false;
        break;

    case 'q': // 프로그램 종료
        glutLeaveMainLoop();    
        break;

    default:
        break;
    }

    glutPostRedisplay();
}