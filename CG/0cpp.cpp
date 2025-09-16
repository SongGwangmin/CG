#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 
#include <random>


std::random_device rd;

// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::mt19937 gen(rd());

// 0 ���� 99 ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_int_distribution<int> dis(0, 256);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Keyboard(unsigned char key, int x, int y);

bool timerRunning = false;

GLdouble Rvalue = 0.0;
GLdouble Gvalue = 0.0;
GLdouble Bvalue = 1.0;


// Ÿ�̸� �ݹ� (glutTimerFunc ��� ȣ��� �Լ� ����)
void TimerFunc(int value) {
    if (!timerRunning) return;

    if (timerRunning) {
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        glutPostRedisplay();
        glutTimerFunc(1000, TimerFunc, 0); // 1�ʸ��� ����
    }
}


void main(int argc, char** argv)
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Example1");
	//--- GLEW �ʱ�ȭ�ϱ�
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

	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡���Եȴ�.
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	//--- �ʿ��� ������� include
	//--- ������ ����ϰ� �ݹ��Լ� ����
   // glut �ʱ�ȭ
   // ���÷��� ��� ����
   // �������� ��ġ ����
   // �������� ũ�� ����
   // ������ ���� (������ �̸�)
	// glew �ʱ�ȭ
   // ��� �Լ��� ����
   // �ٽ� �׸��� �Լ� ����
   // �̺�Ʈ ó�� ����
   //--- �ݹ� �Լ�: ��� �ݹ� �Լ�
   // �������� ��blue�� �� ����
   // ������ ������ ��ü��ĥ�ϱ�
   // ȭ�鿡 ����ϱ�
   //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'c': // û�ϻ� (�ʷ�+�Ķ�)
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'm': // ��ȫ�� (����+�Ķ�)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 0.0;
        Bvalue = 1.0;
        break;

    case 'y': // ����� (����+�ʷ�)
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 0.0;
        break;

    case 'a': // ������
        Rvalue = dis(gen) / 256.0f;
        Gvalue = dis(gen) / 256.0f;
        Bvalue = dis(gen) / 256.0f;
        timerRunning = false;
        break;

    case 'w': // ���
        timerRunning = false;
        Rvalue = 1.0;
        Gvalue = 1.0;
        Bvalue = 1.0;
        break;

    case 'k': // ������
        timerRunning = false;
        Rvalue = 0.0;
        Gvalue = 0.0;
        Bvalue = 0.0;
        break;

    case 't': // Ÿ�̸� ����
        if (!timerRunning) {
            timerRunning = true;
            glutTimerFunc(100, TimerFunc, 0); // 1�ʸ��� ���� �� ����
        }
        break;

    case 's': // Ÿ�̸� ����
        timerRunning = false;
        break;

    case 'q': // ���α׷� ����
        glutLeaveMainLoop();    
        break;

    default:
        break;
    }

    glutPostRedisplay();
}